#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>


//触摸屏默认的设备地址
#define DEFAULT_SLAVE_ADDRESS  0x5d//0x68//0x5d
//触摸屏记录芯片版本的寄存器地址
#define GTP_REG_VERSION        0x8140


int main(int argc, char *argv[])
{
	int res = 0;
	int fd;

	int adapter_nr = 0; 
	char filename[20];


	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
	
	printf("This is the gtxx touch detect demo\n");
	//打开I2C总线的设备文件
	fd = open(filename, O_RDWR);

	if (fd < 0) 
		return -1;
	
	res = ioctl(fd, I2C_SLAVE, DEFAULT_SLAVE_ADDRESS);
	printf("set addr res =%d\n", res);

#if 1

	struct i2c_rdwr_ioctl_data msg;
	struct i2c_msg i2c_msg_data[2];
	char readbuf[8]={GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff};

	i2c_msg_data[0].addr = DEFAULT_SLAVE_ADDRESS;
	i2c_msg_data[0].flags = !I2C_M_RD;
	i2c_msg_data[0].len = 2;
	i2c_msg_data[0].buf = &readbuf[0];

	i2c_msg_data[1].addr = DEFAULT_SLAVE_ADDRESS;
	i2c_msg_data[1].flags = I2C_M_RD;
	i2c_msg_data[1].len = 6;
	i2c_msg_data[1].buf = &readbuf[2];

	msg.nmsgs = 2;
	msg.msgs = i2c_msg_data;

#else
	addbuf[0] = 0x75;
	i2c_msg_data[0].addr = DEFAULT_SLAVE_ADDRESS;
	i2c_msg_data[0].flags = !I2C_M_RD;
	i2c_msg_data[0].len = 1;
	i2c_msg_data[0].buf = addbuf;

	i2c_msg_data[1].addr = DEFAULT_SLAVE_ADDRESS;
	i2c_msg_data[1].flags = I2C_M_RD;
	i2c_msg_data[1].len = 1;
	i2c_msg_data[1].buf = readbuf;
#endif




	//复合读写操作
	res = ioctl(fd, I2C_RDWR, &msg);
	printf("I2C_RDWR res =%d\n", res);

	
	int i;
	for(i=0; i<8; i++){
		printf("buf[%d] = %c\n", i, readbuf[i]);

		printf("buf[%d] = %#x\n", i, readbuf[i]);
	}

	return 0;
}