
#ifndef I2C_MPU6050_H
#define I2C_MPU6050_H


//宏定义
#define SMPLRT_DIV                                  0x19
#define CONFIG                                      0x1A
#define GYRO_CONFIG                                 0x1B
#define ACCEL_CONFIG                                0x1C
#define ACCEL_XOUT_H                                0x3B
#define ACCEL_XOUT_L                                0x3C
#define ACCEL_YOUT_H                                0x3D
#define ACCEL_YOUT_L                                0x3E
#define ACCEL_ZOUT_H                                0x3F
#define ACCEL_ZOUT_L                                0x40
#define TEMP_OUT_H                                  0x41
#define TEMP_OUT_L                                  0x42
#define GYRO_XOUT_H                                 0x43
#define GYRO_XOUT_L                                 0x44
#define GYRO_YOUT_H                                 0x45
#define GYRO_YOUT_L                                 0x46
#define GYRO_ZOUT_H                                 0x47
#define GYRO_ZOUT_L                                 0x48
#define PWR_MGMT_1                                  0x6B
#define WHO_AM_I                                    0x75
#define SlaveAddress                                0xD0
#define Address                                     0x68                  //MPU6050地址
#define I2C_RETRIES                                 0x0701
#define I2C_TIMEOUT                                 0x0702
#define I2C_SLAVE                                   0x0703       //IIC从器件的地址设置
#define I2C_BUS_MODE                                0x0780


#endif