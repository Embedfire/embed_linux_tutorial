/*
 *  bme280.c - Linux device driver for the bme280 humidity,
 *  temperature and barometric sensor.
 *
 *  This driver uses the I2C bus subsystem and creates a char device when
 *  probed to allow interaction with user space. For more details on use 
 *  and implementation please refer to the README file.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * */

#include <linux/module.h> 
#include <linux/i2c.h>
#include "bme280.h"
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <asm/div64.h>

#define DEVICE_NAME "bme280"

/* 
 * Structure that holds bme280 calibration data - this data is loaded into
 * the device at production and it is different for each device, this structure
 * is populated in the probe function. 
 * 
 * */
struct bme280_calibration_t{
	unsigned short dig_T1;
      	signed short dig_T2;
	signed short dig_T3;
	unsigned short dig_P1;
	signed short dig_P2;
	signed short dig_P3;
	signed short dig_P4;
	signed short dig_P5;
	signed short dig_P6;
	signed short dig_P7;
	signed short dig_P8;
	signed short dig_P9;
	unsigned char dig_H1;
	signed short dig_H2;
	unsigned char dig_H3;
	signed short dig_H4;
	signed short dig_H5;
	signed char dig_H6;	
};

/* Structure that holds bme280 configuration */
struct bme280_configuration_t{
	unsigned char ctrl_hum;
	unsigned char ctrl_meas;
	unsigned char config;	
};

/* Structure that holds configuration and client data for the device */
struct bme280_data_t{
	struct i2c_client *client;
	struct bme280_calibration_t *cal_data;
	struct bme280_configuration_t *cfg_data;
};

/* Global variables */
static struct bme280_calibration_t bme280_calibration;
static struct bme280_configuration_t bme280_configuration =
					{
					.ctrl_hum = CTRL_HUM_OSRS_H(1),
					.ctrl_meas = CTRL_MEAS_OSRS_T(1) | CTRL_MEAS_OSRS_P(1),
					.config = 0
					};
struct i2c_client *bme280_client = NULL;
static struct bme280_data_t bme280_data =
					{
					.client = NULL,
					.cal_data = &bme280_calibration,
					.cfg_data = &bme280_configuration
					};
static s32 fine_cal = 0;
static u8 read_flag = 0;
static struct cdev bme280_cdev;
static dev_t device_numbers;

/* 
 * Performs temperature calibration
 *
 *
 * */
static void bme280_calibrate_temp(u32 temp, u32 *cal_temp){

	s32 tmp1, tmp2;

	tmp1 = ((((temp >> 3) - ((s32)bme280_calibration.dig_T1 << 1))) * 
		((s32) bme280_calibration.dig_T2)) >> 11;
	tmp2 = (((((temp >> 4) - ((s32)bme280_calibration.dig_T1)) * 
		((temp >> 4) - ((s32)bme280_calibration.dig_T1))) >> 12) *
		((s32)bme280_calibration.dig_T3)) >> 14;
	fine_cal = tmp1 + tmp2;
	
	*cal_temp = (fine_cal * 5 + 128) >> 8; 
}

/*
 * Performs humidity calibration
 *
 * */
static void bme280_calibrate_hum(s32 hum, u32 *cal_hum){

	s32 tmp1;

	tmp1 = (fine_cal - ((s32)76800));
	tmp1 = ((((hum << 14) - (((s32)bme280_calibration.dig_H4 << 20) -
		(((s32)bme280_calibration.dig_H5) * tmp1)) +
		((s32)16384)) >> 15) * (((((((tmp1 * ((s32)bme280_calibration.dig_H6)) >> 10) *
		(((tmp1 * ((s32)bme280_calibration.dig_H3)) >> 11) + ((s32)32768))) >> 10) + 
		((s32)2097152)) * ((s32)bme280_calibration.dig_H2) + 8192) >> 14));
	tmp1 = (tmp1 - (((((tmp1 >> 15) * (tmp1 >> 15)) >> 7) *
		(( s32)bme280_calibration.dig_H1)) >> 4));
	tmp1 = (tmp1 < 0 ? 0 : tmp1);
	tmp1 = (tmp1 > 419430400 ? 419430400 : tmp1);
	*cal_hum = (u32)(tmp1 >> 12);
}

/*
 * Performs pressure calibration
 *
 * */
static void bme280_calibrate_press(u32 press, u32 *cal_press){
	
	s64 tmp1, tmp2, tmp3, tmp4;

	tmp1 = fine_cal - 128000;
	tmp2 = tmp1 * tmp1 * (s64)bme280_calibration.dig_P6;
	tmp2 = tmp2 + ((tmp1 * (s64)bme280_calibration.dig_P5) << 17);
	tmp2 = tmp2 + (((s64)bme280_calibration.dig_P4) << 35);
	tmp1 = ((tmp1 * tmp1 * (s64)bme280_calibration.dig_P3) >> 8) + 
		((tmp1 * (s64)bme280_calibration.dig_P2) << 12);
	tmp1 = (((((s64)1) << 47) + tmp1)) * ((s64)bme280_calibration.dig_P1) >> 33;
	if(tmp1 == 0){
		*cal_press = 0;
		return;	/* Avoid exception caused by division by zero */
	}
	tmp3 = 1048576 - press;
	tmp3 = (((tmp3 << 31) - tmp2) * 3125);
	tmp4 = do_div(tmp3, tmp1);
	tmp1 = (((s64)bme280_calibration.dig_P9) * (tmp3 >> 13) * (tmp3 >> 13)) >> 25;
	tmp2 = (((s64)bme280_calibration.dig_P8) * tmp3) >> 19;
	tmp3 = ((tmp3 + tmp1 + tmp2) >> 8) + (((s64)bme280_calibration.dig_P7) << 4);
	*cal_press = (u32)tmp3;
}

/* Helper functions definitions */
/*
 * This helper function is used to obtain the calibration
 * parameters and populate the calibration structure
 *
 * parameter | Register address |   bit
 * ----------|------------------|----------------
 * dig_T1    |  0x88 and 0x89   | from 0 : 7 to 8: 15
 * dig_T2    |  0x8A and 0x8B   | from 0 : 7 to 8: 15
 * dig_T3    |  0x8C and 0x8D   | from 0 : 7 to 8: 15
 * dig_P1    |  0x8E and 0x8F   | from 0 : 7 to 8: 15
 * dig_P2    |  0x90 and 0x91   | from 0 : 7 to 8: 15
 * dig_P3    |  0x92 and 0x93   | from 0 : 7 to 8: 15
 * dig_P4    |  0x94 and 0x95   | from 0 : 7 to 8: 15
 * dig_P5    |  0x96 and 0x97   | from 0 : 7 to 8: 15
 * dig_P6    |  0x98 and 0x99   | from 0 : 7 to 8: 15
 * dig_P7    |  0x9A and 0x9B   | from 0 : 7 to 8: 15
 * dig_P8    |  0x9C and 0x9D   | from 0 : 7 to 8: 15
 * dig_P9    |  0x9E and 0x9F   | from 0 : 7 to 8: 15
 * dig_H1    |         0xA1     | from 0 to 7
 * dig_H2    |  0xE1 and 0xE2   | from 0 : 7 to 8: 15
 * dig_H3    |         0xE3     | from 0 to 7
 * dig_H4    |0xE4 and 0xE5[3:0]| from 4 : 11 to 0: 3
 * dig_H5    |0xE5[7:4] and 0xE6| from 0 : 3 to 4: 11
 * dig_H6    |         0xE7     | from 0 to 7
 * 
 * */
static int bme280_get_calibration(struct bme280_calibration_t *calibration){
	u8 calib_data_0_25[CALIBRATION_REG_SIZE_0];
	u8 calib_data_26_41[CALIBRATION_REG_SIZE_26];
	s32 tmp;
	u8 i = 0;

	/* 
	 * Read calibration data, the data is arranged in two chunks
	 * from 0x88 (calib0) to 0xA1 (calib25) and 
	 * from 0xE1 (calib26) to 0xF0 (calib41)
	 * Use read byte instead of emulated block data read because 4.1 
	 * does not support emulated block read
	 * */
	while(i < CALIBRATION_REG_SIZE_0){
		tmp = i2c_smbus_read_byte_data(bme280_data.client, R_BME280_CALIB00 + i);
		if(tmp < 0){
			goto out;
		}
		calib_data_0_25[i] = (u8) (tmp & 0xFF);
		i++;
	}
	i = 0;
	while(i < CALIBRATION_REG_SIZE_26){
		tmp = i2c_smbus_read_byte_data(bme280_data.client, R_BME280_CALIB26 + i);
		if(tmp < 0){
			goto out;
		}
		calib_data_26_41[i] = (u8) (tmp & 0xFF);
		i++;
	}

	/* Fill calibration structure from obtained parameters */
	bme280_data.cal_data->dig_T1 = (unsigned short)((calib_data_0_25[1] << 8) | calib_data_0_25[0]);
	bme280_data.cal_data->dig_T2 = (short) ((calib_data_0_25[3] << 8) | calib_data_0_25[2]);	
	bme280_data.cal_data->dig_T3 = (short) ((calib_data_0_25[5] << 8) | calib_data_0_25[4]);
	bme280_data.cal_data->dig_P1 = (unsigned short) ((calib_data_0_25[7] << 8) | calib_data_0_25[6]);
	bme280_data.cal_data->dig_P2 = (short) ((calib_data_0_25[9] << 8) | calib_data_0_25[8]);
	bme280_data.cal_data->dig_P3 = (short) ((calib_data_0_25[11] << 8) | calib_data_0_25[10]);
	bme280_data.cal_data->dig_P4 = (short) ((calib_data_0_25[13] << 8) | calib_data_0_25[12]);
	bme280_data.cal_data->dig_P5 = (short) ((calib_data_0_25[15] << 8) | calib_data_0_25[14]);
	bme280_data.cal_data->dig_P6 = (short) ((calib_data_0_25[17] << 8) | calib_data_0_25[16]);
	bme280_data.cal_data->dig_P7 = (short) ((calib_data_0_25[19] << 8) | calib_data_0_25[18]);
	bme280_data.cal_data->dig_P8 = (short) ((calib_data_0_25[21] << 8) | calib_data_0_25[20]);
	bme280_data.cal_data->dig_P9 = (short) ((calib_data_0_25[23] << 8) | calib_data_0_25[22]);
	bme280_data.cal_data->dig_H1 = calib_data_0_25[24];
	bme280_data.cal_data->dig_H2 = (short) ((calib_data_26_41[1] << 8) | calib_data_26_41[0]);
	bme280_data.cal_data->dig_H3 = calib_data_26_41[2];
	bme280_data.cal_data->dig_H4 = (short) ((calib_data_26_41[4] & 0x0F ) | 
			(calib_data_26_41[3] << 4));
	bme280_data.cal_data->dig_H5 = (short) ((calib_data_26_41[5] << 4) | 
			( (calib_data_26_41[4] & 0xF0) >> 4 ) );
	bme280_data.cal_data->dig_H6 = (s8) calib_data_26_41[6];
	
	return 0;
out:
	return tmp;
}

static int bme280_set_configuration(struct bme280_configuration_t *configuration){
	int tmp;

	/* Configure CTRL_HUM  */
	tmp = i2c_smbus_write_byte_data(bme280_data.client, R_BME280_CTRL_HUM, 
			bme280_data.cfg_data->ctrl_hum);
	if(tmp < 0){
		printk(KERN_INFO "%s: Unable to write CTRL_HUM.\n", DEVICE_NAME);
		goto out;
	}
		
	/* 
	 * Configure CTRL_MEAS  
	 * changes to CTRL_HUM become effective only after writing to CTRL_MEAS 
	 * */
	tmp = i2c_smbus_write_byte_data(bme280_data.client, R_BME280_CTRL_MEAS,
			bme280_data.cfg_data->ctrl_meas);
	if(tmp < 0){
		printk(KERN_INFO "%s: Unable to write CTRL_MEAS.\n", DEVICE_NAME);
		goto out;
	}

	/* Set CONFIG register */
	tmp = i2c_smbus_write_byte_data(bme280_data.client, R_BME280_CONFIG,
			bme280_data.cfg_data->config);
	if(tmp < 0){
		printk(KERN_INFO "%s: Unable to write CONFIG.\n", DEVICE_NAME);
		goto out;
	}

out:
	return tmp;
}


/* Char device declarations */
static int bme280_open(struct inode *inode, struct file *filp){
	read_flag = 0;
	return 0;
}

static int bme280_release(struct inode *inode, struct file *filp){
	read_flag = 0;
	return 0;
}

static ssize_t bme280_read(struct file *filp, char *buf, size_t count, loff_t *ppos){
	int tmp, working_mode;
	u32 i = 0;
	u8 press_st[MAX_STRING_SIZE];
	u8 temp_st[MAX_STRING_SIZE];
	u8 hum_st[MAX_STRING_SIZE];
	u8 final_st[MAX_STRING_SIZE * 3];
	u32 press, temp;
	u16 hum;
	s32 cal_temp, cal_hum, cal_press;
	u8 data_readout[DATA_READOUT_SIZE];
	
	/* Check if data was read already */
	if(read_flag == 1){
		read_flag = 0;
		return 0;
	}

	/* Check working mode */
	working_mode = i2c_smbus_read_byte_data(bme280_data.client, R_BME280_CTRL_MEAS);
	if(working_mode < 0){
		tmp = working_mode;
		goto out;
	}
	
	/* If in sleep mode we need to do a force read */
	if((working_mode & 0b11) == 0){
		tmp = i2c_smbus_write_byte_data(bme280_data.client, R_BME280_CTRL_MEAS, 
				(working_mode | 0b10));
		if(tmp < 0){
			goto out;
		}
	}

	/* Check status flag */
	do{
		tmp = i2c_smbus_read_byte_data(bme280_data.client, R_BME280_STATUS);
		if(tmp < 0){
			goto out;
		}
	}while(tmp & STATUS_MEASURING_RUNNING);

	/* 
	 * Perform raw read, it is adviced to perform a burst read
	 * from 0xF7 (PRESS_MSB) to 0xFE (HUM_LSB)
	 * Use read byte instead of emulated block data read because 4.1 
	 * does not support emulated block read
	 * */
	while(i < DATA_READOUT_SIZE){
		tmp = i2c_smbus_read_byte_data(bme280_data.client, R_BME280_PRESS_MSB + i);
		if(tmp < 0){
			goto out;
		}
		data_readout[i] =(u8) (tmp & 0xFF);
		i++;
	}

	/* Process raw data */
	press = (data_readout[0] << 12) | (data_readout[1] << 4) | (data_readout[2] >> 4);
	temp = (data_readout[3] << 12) | (data_readout[4] << 4) | (data_readout[5] >> 4);
	hum = (data_readout[6] << 8) |  data_readout[7];

	/* Compensate obtained results
	 *
	 * Temperature must be compensated first, a value that is required
	 * by the other parameters is calculated by the temperature 
	 * calbration routine.
	 * */
	bme280_calibrate_temp(temp, &cal_temp);
	bme280_calibrate_hum(hum, &cal_hum);
	bme280_calibrate_press(press, &cal_press);

	/* Form string */
	/* Turn numeric values to strings */
	tmp = snprintf(press_st, MAX_STRING_SIZE, "%d", cal_press);
	if(tmp >= MAX_STRING_SIZE){
		printk(KERN_INFO "%s: Pressure string truncated\n", DEVICE_NAME);
	}
	
	tmp = snprintf(temp_st, MAX_STRING_SIZE, "%d", cal_temp);
	if(tmp >= MAX_STRING_SIZE){
		printk(KERN_INFO "%s: Temperature string truncated\n", DEVICE_NAME);
	}
	
	tmp = snprintf(hum_st, MAX_STRING_SIZE, "%d", cal_hum);
	if(tmp >= MAX_STRING_SIZE){
		printk(KERN_INFO "%s: Humidity string truncated\n", DEVICE_NAME);
	}

	/* Create string */
	strcpy(final_st, "t");
	strcat(final_st, temp_st);
	strcat(final_st, "p");
	strcat(final_st, press_st);
	strcat(final_st, "h");
	strcat(final_st, hum_st);
	strcat(final_st, "\n");

	/* Copy from kernel space to user space */
	tmp = copy_to_user(buf, final_st, strlen(final_st));
	read_flag = 1;
	return strlen(final_st);
out:
	return tmp;
}

static struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = bme280_read,
		.open = bme280_open,
		.release = bme280_release,
};

/*
 * When the device is probed this function performs the following actions:
 *  -Reads device ID - NOTE: I2C Address should be configured in the device tree (or the method
 *                     used to register the device)
 *  -Reads calibration parameters
 *  -Configures device
 *  -Registers char device for user space communication
 * */
static int bme280_probe(struct i2c_client *client, const struct i2c_device_id *id){
	
	int tmp, client_id;
	struct i2c_adapter *adapter;
	
	/* Check if adapter supports the functionality we need */
	adapter = client->adapter;
	tmp = i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA);
	if(!tmp)
		goto err_out;

	/* Get chip_id */
	client_id = i2c_smbus_read_byte_data(client, R_BME280_CHIP_ID);
	if(client_id != BME280_CHIP_ID){
		printk(KERN_INFO "%s: Client ID (%x) does not match chip ID (%x).\n", 
			DEVICE_NAME, client_id, BME280_CHIP_ID);
		goto err_out;
	}
	bme280_data.client = client;

	/* Get calibration parameters */
	bme280_get_calibration(bme280_data.cal_data);

	/* Set configuration */
	tmp = bme280_set_configuration(bme280_data.cfg_data);
	if(tmp < 0){
		printk(KERN_INFO "%s: Unable to configure device.\n", DEVICE_NAME);
		goto err_out;
	} 

	/* Register char device */
	tmp = alloc_chrdev_region(&device_numbers, 0, 1, DEVICE_NAME);
	if(tmp < 0){
		printk(KERN_INFO "%s: Unable to register char device.\n", DEVICE_NAME);
		goto err_out;
	}

	/* Creating character device (cdev) "object" */
	cdev_init(&bme280_cdev, &fops);
	bme280_cdev.owner = THIS_MODULE;
	bme280_cdev.ops = &fops;
	tmp = cdev_add(&bme280_cdev, device_numbers, 1);
	if(tmp){
		printk(KERN_ALERT "%s: Unable to add cdev, with errno %d.\n", DEVICE_NAME, tmp);
		goto err_out2;
	}
	
	printk(KERN_INFO "%s: Probe executed successfully.\n", DEVICE_NAME);
err_out:
	return tmp;
err_out2:
	unregister_chrdev_region(device_numbers, 1);
	return tmp;
}

static int bme280_remove(struct i2c_client *client){
	/* Unregister char device */
	unregister_chrdev_region(device_numbers, 1);
	return 0;
}

static struct i2c_device_id bme280_idtable[] = {
	{DEVICE_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, bme280_idtable);

static struct i2c_driver bme280_driver = {
	.driver = {
		    .name = DEVICE_NAME,
		    .owner = THIS_MODULE,
		  },
	.probe = bme280_probe,
	.remove = bme280_remove,
	.id_table = bme280_idtable
};

/* Module init routine */
static int __init bme280_init(void){
	/* Register driver within I2C subsystem */
	int tmp;
	tmp = i2c_add_driver(&bme280_driver);
	if(tmp < 0){
		printk(KERN_INFO "%s: I2C subsystem registration failed.\n", DEVICE_NAME);
	}
	printk(KERN_INFO "%s: Device successfully registered to I2C subsystem.\n", DEVICE_NAME);
	return tmp;
}

/* Module exit routine */
static void __exit bme280_exit(void){

	/* Unregister driver from I2C subsystem */
	i2c_del_driver(&bme280_driver);
}

module_init(bme280_init);
module_exit(bme280_exit);

MODULE_AUTHOR("Manuel Rodriguez <manuel2982@gmail.com>");
MODULE_DESCRIPTION("Driver for BME280 sensor");
MODULE_LICENSE("GPL");
