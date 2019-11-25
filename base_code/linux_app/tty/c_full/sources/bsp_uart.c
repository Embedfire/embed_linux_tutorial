#include <stdio.h>                          //standard input & output，标准输入输出，经常使用的printf函数，scanf函数
#include <stdlib.h>                         //standard library，标准输入输出库，包含malloc函数、free函数
#include <unistd.h>                         //POSIX系统接口头文件
#include <fcntl.h>                          //文件接口头文件
#include <sys/types.h>                      //系统基础数据类型
#include <sys/stat.h>                       //定义文件的状态
#include <termios.h>                        //Linux串口头文件
#include <errno.h>                          //错误码头文件，定义了一些错误宏定义


#include "bsp_uart.h"


/**
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return  void
*/
int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
					B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
					19200,  9600, 4800, 2400, 1200,  300, };
void set_speed(int fd, int speed)
{
	int   i; 
	int   status; 
	struct termios   optinos;
	tcgetattr(fd, &optinos); 
	for ( i= 0; i < sizeof(speed_arr)/sizeof(int); i++) { 
		if  (speed == name_arr[i]) {     
		tcflush(fd, TCIOFLUSH);     
		cfsetispeed(&optinos, speed_arr[i]);  
		cfsetospeed(&optinos, speed_arr[i]);   
		status = tcsetattr(fd, TCSANOW, &optinos); 

		if  (status != 0) {        
			perror("tcsetattr fd");  
			return;     
		}
		tcflush(fd,TCIOFLUSH);   
		}
  }
}

/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄
*@param  databits 类型  int 数据位   取值 为5,6,7,8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity  类型  int  效验类型 取值为N,E,O,S
*/
int set_parity(int fd,int databits,int stopbits,int parity)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;  
	}
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*设置数据位数*/
	{   
		case 5:     
			options.c_cflag |= CS5; 
			break;
		case 6:     
			options.c_cflag |= CS6;
			break;   
		case 7:     
			options.c_cflag |= CS7; 
			break;
		case 8:     
			options.c_cflag |= CS8;
			break;   
		default:    
			fprintf(stderr,"Unsupported data size\n"); return  -1;  
	}
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* 不使用奇偶校验 */
			options.c_iflag &= ~INPCK;     /* 禁止输入奇偶检测 */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= PARENB; 	/* 启用奇偶效验 */  
			options.c_iflag |= INPCK;   /* 启用输入奇偶检测 */ 
			options.c_cflag |= PARODD ; /* 设置为奇效验 */
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;    /* 启用奇偶效验 */    
			options.c_iflag |= INPCK;     /* 启用输入奇偶检测 */
			options.c_cflag &= ~PARODD;   /* 设置为偶效验*/     
			break;
		case 'S': 
		case 's':  /*as no parity*/   
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;break;  
		default:   
			fprintf(stderr,"Unsupported parity\n");    
			return  -1;  
		}  
	/* 设置停止位*/  
	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
		break;
		default:    
			fprintf(stderr,"Unsupported stop bits\n");  
			return  -1; 
	} 

	// 如果不是开发终端之类的，只是串口传输数据，而不需要串口来处理，
	// 那么使用原始模式(Raw Mode)方式来通讯，设置方式如下
	// options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	// options.c_oflag  &= ~OPOST;   /*Output*/

	/* Set input parity option */ 
	if (parity != 'n')   
		options.c_iflag |= INPCK; 
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	
	if (tcsetattr(fd,TCSANOW,&options) != 0)   
	{ 
		perror("SetupSerial 3");   
		return  -1;  
	} 
	return 0;  
}


