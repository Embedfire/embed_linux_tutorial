/*
 * sht3x.c
 * Library for sht3x temperature sensor.
 *
 * Copyright (c) 2018 seeed technology inc.
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/kdev_t.h>
#include <linux/sysfs.h>

/**************************************************************************************************************/
/**************************************************************************************************************/



#define  MAX_NAME_LEN   30
#define AMPLIFICAT_NUM   100


static int dev_cnt = -1;
struct sht3x_dev *sht3x_info_buf[256];


struct sht3x_dev
{
	int major;
	struct class *sht3x_cls;
	struct device *sht3x_dev;
	char class_name[MAX_NAME_LEN];
	char device_name[MAX_NAME_LEN];
	struct i2c_client *client;
	int32_t humidity;
	int32_t temperature;
};


/*********************************************sht3x api***********************************************************/

static uint8_t sht3x_crc8(const uint8_t *data, int len)
{
	int i,j;
	const uint8_t POLYNOMIAL=0x31;
	uint8_t crc=0xFF;


	for (  j = len; j; --j ) {
	crc ^= *data++;

		for ( i = 8; i; --i ) {
			crc = ( crc & 0x80 )
				? (crc << 1) ^ POLYNOMIAL
				: (crc << 1);
		}
	}
	return crc;
}



static int32_t sht3x_read_temperature(struct i2c_client *client)
{
	uint8_t pdata = 0x00;
	uint8_t rdata[6] = {0};
	struct sht3x_dev *sht3x_info = NULL;
	sht3x_info = i2c_get_clientdata(client);
	if(i2c_smbus_write_i2c_block_data(client,0x24,1,&pdata)){
		printk(KERN_ALERT "Smbus send bytes failed\n");
		
		return -1;
	}
	msleep(20);
	if(i2c_smbus_read_i2c_block_data(client,0x00,6,rdata) < 0){
		printk(KERN_ALERT "Smbus read bytes failed\n");
		return -1;
	}
	if((rdata[2] != sht3x_crc8(rdata,2)) || (rdata[5] != sht3x_crc8(&rdata[3],2)) ){
		printk(KERN_ALERT "Crc failed!\n");
		return -2;
	}

	sht3x_info->temperature = rdata[0] * 256 + rdata[1];
	sht3x_info->temperature = -45 * AMPLIFICAT_NUM + (175 * sht3x_info->temperature *AMPLIFICAT_NUM / 65535);
	sht3x_info->humidity = AMPLIFICAT_NUM * (rdata[3]* 256 + rdata[4]) * AMPLIFICAT_NUM / 65535;
	// printk(KERN_INFO "read temperature = %d\n",sht3x_info->temperature);
	// printk(KERN_INFO "read humidity = %d\n",sht3x_info->humidity);	
	return 0;
}



/***************************************************** sysfs interface *****************************************/
/***************************************************** sysfs interface *****************************************/
/***************************************************** sysfs interface *****************************************/

static ssize_t sht3x_temperature_show(struct kobject* kobjs,struct kobj_attribute *attr,char *buf)
{
	struct i2c_client *client = NULL;
	uint8_t dis_buf[50] = {0};

	client = sht3x_info_buf[dev_cnt]->client;

	if(sht3x_read_temperature(client)){
		sprintf(dis_buf,"Read sensor failed\n");
		return 0;
	}

	else {
		return sprintf(buf,"temperature = %d.%d℃ \nhumidity = %d.%d%% \n\n",
			sht3x_info_buf[dev_cnt]->temperature / AMPLIFICAT_NUM,
			sht3x_info_buf[dev_cnt]->temperature % AMPLIFICAT_NUM,
			sht3x_info_buf[dev_cnt]->humidity / AMPLIFICAT_NUM,
			sht3x_info_buf[dev_cnt]->humidity % AMPLIFICAT_NUM);
	}
	return 0;
}
static struct kobj_attribute status_attr = __ATTR_RO(sht3x_temperature);


/*************************************************** file operations ********************************************/
/*************************************************** file operations ********************************************/
/*************************************************** file operations ********************************************/
static int sht3x_i2c_open(struct inode *node, struct file *file)
{
	return 0;
}

static ssize_t sht3x_i2c_read(struct file *file,char *buf, size_t len,loff_t *offset)
{
	struct i2c_client *client = NULL;
	uint32_t buf_len = 0;
	uint8_t dis_buf[50] = {0};

	client = sht3x_info_buf[dev_cnt]->client;

	if(sht3x_read_temperature(client)){
		sprintf(dis_buf,"Read sensor failed\n");
	}

	else {
		sprintf(dis_buf,"temperature = %d.%d℃ \nhumidity = %d.%d%% \n\n",
		sht3x_info_buf[dev_cnt]->temperature / AMPLIFICAT_NUM,
		sht3x_info_buf[dev_cnt]->temperature % AMPLIFICAT_NUM,
		sht3x_info_buf[dev_cnt]->humidity / AMPLIFICAT_NUM,
		sht3x_info_buf[dev_cnt]->humidity % AMPLIFICAT_NUM);
	}
	buf_len = strlen(dis_buf);
	if(copy_to_user(buf,dis_buf,strlen(dis_buf))){
		printk(KERN_INFO "copy to user failed\n");
		return -ENOMEM;
	}
	return buf_len;
}


static struct file_operations file_oprts = {
	.open = sht3x_i2c_open,
	.read = sht3x_i2c_read,
};



static void sht3x_dev_init(struct sht3x_dev *sht3x_info)
{
	sprintf(sht3x_info->class_name,"%s%d","sht3x_cls",dev_cnt);
	sprintf(sht3x_info->device_name,"%s%d","sht3x_dev",dev_cnt);
}


static int sht3x_drv_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sht3x_dev *sht3x_info = NULL;
	// printk(KERN_INFO "Probe!\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "i2c_check_functionality error\n");
		return -EIO;
	}

	sht3x_info = kmalloc(sizeof(*sht3x_info),GFP_KERNEL);    
	if(NULL == sht3x_info){
		return -ENOMEM;
	}

	sht3x_info_buf[++dev_cnt] = sht3x_info;

	sht3x_dev_init(sht3x_info);

	sht3x_info->client = client;
	i2c_set_clientdata(client,sht3x_info);
	// dev_set_drvdata(&client->dev,sht3x_info);

	sht3x_read_temperature(client);


	if(sysfs_create_file(&sht3x_info->client->dev.kobj, &status_attr.attr) < 0){
		printk(KERN_ALERT "Fail to create sys file\n");
		kfree(sht3x_info);
		return -ENOMEM;
    	}

	sht3x_info->major = register_chrdev(0,sht3x_info->device_name,&file_oprts);
	if(sht3x_info->major < 0 ){
		
		sysfs_remove_file(&sht3x_info->client->dev.kobj, &status_attr.attr);
		printk(KERN_ALERT "Register failed!!\r\n");
		kfree(sht3x_info);
		return sht3x_info->major;
	}
	// printk(KERN_ALERT "Registe success,sht3x_info->major number = %d\r\n",sht3x_info->major);

	sht3x_info->sht3x_cls = class_create(THIS_MODULE,sht3x_info->class_name);
	if(IS_ERR(sht3x_info->sht3x_cls)){
		unregister_chrdev(sht3x_info->major,sht3x_info->device_name);
		sysfs_remove_file(&sht3x_info->client->dev.kobj, &status_attr.attr);
		kfree(sht3x_info);
		return PTR_ERR(sht3x_info->sht3x_cls);
	}

	sht3x_info->sht3x_dev = device_create(sht3x_info->sht3x_cls,NULL,MKDEV(sht3x_info->major,0),NULL,sht3x_info->device_name);
	if(IS_ERR(sht3x_info->sht3x_dev)){
		
		sysfs_remove_file(&sht3x_info->client->dev.kobj, &status_attr.attr);
		class_destroy(sht3x_info->sht3x_cls);
		unregister_chrdev(sht3x_info->major,sht3x_info->device_name);
		kfree(sht3x_info);
		return PTR_ERR(sht3x_info->sht3x_dev);
	}
    	// printk(KERN_ALERT "sht3x_i2c device init success!!\r\n");
    
    	return 0;
}

static int sht3x_drv_remove(struct i2c_client *client)
{
	struct sht3x_dev *sht3x_info = NULL;
	sht3x_info = i2c_get_clientdata(client);

	sysfs_remove_file(&sht3x_info->client->dev.kobj, &status_attr.attr);
	device_destroy(sht3x_info->sht3x_cls,MKDEV(sht3x_info->major,0));
	class_unregister(sht3x_info->sht3x_cls);
	class_destroy(sht3x_info->sht3x_cls);
	unregister_chrdev(sht3x_info->major,sht3x_info->device_name);
	kfree(sht3x_info);
    	return 0;
}


static const struct i2c_device_id sht3x_drv_id_table[] = {
	{"sht3x",0},
	{},
};

static const struct of_device_id sht3x_match_table[] = {
	{.compatible = "sensirion,sht3x"},
	{},
};


static struct i2c_driver sht3x_drv = {
	.driver = {
		.name = "sht3x",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sht3x_match_table),
	},
	.probe = sht3x_drv_probe,
	.remove = sht3x_drv_remove,
	.id_table = sht3x_drv_id_table,
};

MODULE_DEVICE_TABLE(of,sht3x_match_table);


int drv_init(void)
{
	int ret = 0;
	ret  = i2c_add_driver(&sht3x_drv);
	if(ret){
		printk(KERN_ALERT "add driver failed!!!\n");
		return -ENODEV;
	}
	return 0;
}


void drv_exit(void)
{
	i2c_del_driver(&sht3x_drv);
	return ;
}


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This module is for Seeed sht3x ranging sensor.");
MODULE_AUTHOR("Downey");

module_init(drv_init);
module_exit(drv_exit);
