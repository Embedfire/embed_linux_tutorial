
/************************************************************/
//文件名：mpu6050.c
//功能:测试linux下iic读写mpu6050程序
//使用说明: (1)
//          (2)
//          (3)
//          (4)
//作者:pengjie
//日期:2019-07-03

/************************************************************/
//包含头文件
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/select.h>
#include<sys/time.h>
#include<errno.h>
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

typedef unsigned char uint8;
int fd = -1;

//函数声明
static uint8 MPU6050_Init(void);
static uint8 i2c_write(int fd, uint8 reg, uint8 val);
static uint8 i2c_read(int fd, uint8 reg, uint8 * val);
static uint8 printarray(uint8 Array[], uint8 Num);

//MPU6050初始化
static uint8 MPU6050_Init(void)
{
    fd  = open("/dev/i2c-0", O_RDWR);               // open file and enable read and  write

    if (fd < 0)
    {
        perror("Can't open /dev/MPU6050 \n");       // open i2c dev file fail
        exit(1);
    }

    printf("open /dev/i2c-0 success !\n");          // open i2c dev file succes

    if (ioctl(fd, I2C_SLAVE, Address) < 0)
    { //set i2c address
        printf("fail to set i2c device slave address!\n");
        close(fd);
        return - 1;
    }

    printf("set slave address to 0x%x success!\n", Address);
    i2c_write(fd, PWR_MGMT_1, 0X00);
    i2c_write(fd, SMPLRT_DIV, 0X07);
    i2c_write(fd, CONFIG, 0X06);
    i2c_write(fd, ACCEL_CONFIG, 0X01);
    return (1);
}

//MPU6050 wirte byte
static uint8 i2c_write(int fd, uint8 reg, uint8 val)
{
    int retries;
    uint8 data[2];

    data[0] = reg;
    data[1] = val;

    for (retries = 5; retries; retries--)
    {
        if (write(fd, data, 2) == 2)
        {
            return 0;

        }

        usleep(1000 * 10);
    }

    return - 1;
}

//MPU6050 read byte
static uint8 i2c_read(int fd, uint8 reg, uint8 * val)
{
    int retries;

    for (retries = 5; retries; retries--)
    {
        if (write(fd, &reg, 1) == 1)
        {
            if (read(fd, val, 1) == 1)
            {
                return 0;
            }

        }

    }

    return - 1;
}

//get data
short GetData(unsigned char REG_Address)
{
    char H, L;

    i2c_read(fd, REG_Address, &H);
    usleep(1000);
    i2c_read(fd, REG_Address + 1, &L);
    return (H << 8) +L;
}

// main
int main(int argc, char * argv[])
{
    MPU6050_Init();
    usleep(1000 * 100);

    while (1)
    {
        //printf("\033[2J");
        usleep(1000 * 200);
        printf("ACCE_X:%6d\n ", GetData(ACCEL_XOUT_H));
        usleep(1000 * 200);
        printf("ACCE_Y:%6d\n ", GetData(ACCEL_YOUT_H));
        usleep(1000 * 200);
        printf("ACCE_Z:%6d\n ", GetData(ACCEL_ZOUT_H));
        usleep(1000 * 200);
        printf("GYRO_X:%6d\n ", GetData(GYRO_XOUT_H));
        usleep(1000 * 200);
        printf("GYRO_Y:%6d\n ", GetData(GYRO_YOUT_H));
        usleep(1000 * 200);
        printf("GYRO_Z:%6d\n\n ", GetData(GYRO_ZOUT_H));
        sleep(1);
    }

    close(fd);
}
