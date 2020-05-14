/*
 *  bme280.h - Device definitions for the bme280 humidity,
 *  temperature and barometric sensor.
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


/* BME280 registers - for details please refer to the device DS */
#define R_BME280_CALIB00		0x88
#define R_BME280_CALIB25		0xA1
#define R_BME280_CHIP_ID		0xD0
#define R_BME280_RESET			0xE0
#define R_BME280_CALIB26			0xE1
#define R_BME280_CALIB41			0xF0
#define R_BME280_CTRL_HUM			0xF2
#define R_BME280_STATUS				0xF3
#define R_BME280_CTRL_MEAS		0xF4
#define R_BME280_CONFIG			0xF5
#define R_BME280_PRESS_MSB		0xF7
#define R_BME280_PRESS_LSB		0xF8
#define R_BME280_PRESS_XLSB			0xF9
#define R_BME280_TEMP_MSB			0xFA
#define R_BME280_TEMP_LSB			0xFB
#define R_BME280_TEMP_XLSB			0xFC
#define R_BME280_HUM_MSB		0xFD
#define R_BME280_HUM_LSB		0xFE

/* BME280 definitions */
#define BME280_CHIP_ID			0x60
#define BME280_I2C_ADDR			0x77

/* Configuration fields */
#define CTRL_HUM_OSRS_H(x)		(x)			
#define CTRL_MEAS_MODE(x)		(x)
#define CTRL_MEAS_OSRS_P(x)		(x << 2)
#define CTRL_MEAS_OSRS_T(x)		(x << 5)
#define	CONFIG_T_SB(x)			(x << 5)
#define CONFIG_FILTER(x)		(x << 2)
#define CONFIG_SPI3W_EN(x)		(x)
#define STATUS_MEASURING_RUNNING	(1 << 3)

/* Data readout size */
#define DATA_READOUT_SIZE	8
#if(DATA_READOUT_SIZE > I2C_SMBUS_BLOCK_MAX)
#error "Data readout is greater than I2C_SMBUS_BLOCK_MAX."
#endif
#define MAX_STRING_SIZE		10
#define CALIBRATION_REG_SIZE_0	26
#define CALIBRATION_REG_SIZE_26 16
/* IOCTL Commands definitions */

