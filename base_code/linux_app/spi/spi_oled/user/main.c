/************************************************************/
//功能:测试linux下spi读写OLED显示屏程序
//使用说明: (1)
//          (2)
//          (3)
//          (4)
//日期:2019-07-03

/************************************************************/



/*SPI OLED 接线说明
* OLED D/C   引脚， CSI_HSYNC   CN5 IO4.20 34    0 指令，1 数据
* OLED MOSI  引脚， UART2_CTS   CN4 IO1.22 50
* OLED CLOCK 引脚， UART2_RXD   CN4 IO1.21 52
* OLED CS    引脚， GND
* OLED GND   引脚， GND
* OLED VCC   引脚， 3.3V  OLED不兼容5V
*/


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "spi_oled_app.h"



/* 设备文件描述符 */
extern int data_or_command; // 命令 、 数据 控制引脚的设备文件描述符
extern int fd;				// SPI 控制引脚的设备文件描述符

/*程序中使用到的点阵 字库*/
extern const unsigned char BMP1[];


int main(int argc, char *argv[])
{
	int i = 0; //用于循环
	oled_init();
	OLED_Fill(0xFF);


	while (1)
	{
		sleep(1);

		OLED_Fill(0x00); //清屏
		sleep(1);

		OLED_ShowStr(0, 3, (unsigned char *)"Wildfire Tech", 1);  //测试6*8字符
		OLED_ShowStr(0, 4, (unsigned char *)"Hello wildfire", 2); //测试8*16字符
		sleep(1);
		OLED_Fill(0x00); //清屏

		for (i = 0; i < 4; i++)
		{
			OLED_ShowCN(22 + i * 16, 0, i); //测试显示中文
		}
		sleep(1);
		OLED_Fill(0x00); //清屏

		OLED_DrawBMP(0, 0, 128, 8, (unsigned char *)BMP1); //测试BMP位图显示
		sleep(1);
		OLED_Fill(0x00); //清屏
	}

	close(fd);

	return 0;
}

