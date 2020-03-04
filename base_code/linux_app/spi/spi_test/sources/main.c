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


/*SPI 接收 、发送 缓冲区*/
unsigned char tx_buffer[100] = "hello the world !";
unsigned char rx_buffer[100];


extern int fd;                  // SPI 控制引脚的设备文件描述符



int main(int argc, char *argv[])
{
	/*初始化SPI */
	spi_init();

	/*执行发送*/
	transfer(fd, tx_buffer, rx_buffer, sizeof(tx_buffer));

	/*打印 tx_buffer 和 rx_buffer*/
	printf("tx_buffer: \n %s\n ", tx_buffer);
	printf("rx_buffer: \n %s\n ", rx_buffer);

	close(fd);

	return 0;
}
