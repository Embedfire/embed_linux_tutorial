#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>
#include "bsp_uart.h"

//根据具体的设备修改
const char default_path[] = "/dev/ttymxc2";
// const char default_path[] = "/dev/ttymxc2";


int main(int argc, char *argv[])
{
	int fd;
	int res;
	struct termios opt;
	char *path;
	char buf[1024] = "Embedfire tty send test.\n";

	//若无输入参数则使用默认终端设备
	if(argc > 1)
		path = argv[1];
	else
		path = (char *)default_path;

	//获取串口设备描述符
	printf("This is tty/usart demo.\n");
	fd = open(path, O_RDWR);
	if(fd < 0){
		printf("Fail to Open %s device\n", path);
		return 0;
	}

	set_speed(fd, 9600);
	set_parity(fd, 8, 1, 'N');
	printf("Device %s is set to 9600bps,8N1.\n9600bps!\n9600bps!\n9600bps!\n",path);

	do{
		//发送字符串
		write(fd, buf, strlen(buf));
		//接收字符串
		res = read(fd, buf, 1024);		
		if(res >0 ){
			//给接收到的字符串加结束符
			buf[res] = '\0';
			printf("Receive res = %d bytes data: %s\n",res, buf);
		}
	}while(res >= 0);

	printf("read error,res = %d",res);

	close(fd);
	return 0;
}