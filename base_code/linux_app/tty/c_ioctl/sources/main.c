#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

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
	//清空串口接收缓冲区
	tcflush(fd, TCIOFLUSH);
	// 获取串口参数opt
	// tcgetattr(fd, &opt);

	res = ioctl(fd,TCGETS, &opt);

	opt.c_ispeed = opt.c_cflag & (CBAUD | CBAUDEX);
	opt.c_ospeed = opt.c_cflag & (CBAUD | CBAUDEX);

	//输出宏定义的值，方便对比
	printf("Macro B9600 = %#o\n",B9600);
	printf("Macro B115200 = %#o\n",B115200);
	//输出读取到的值
  	printf("ioctl TCGETS,opt.c_ospeed = %#o\n", opt.c_ospeed);
	printf("ioctl TCGETS,opt.c_ispeed = %#o\n", opt.c_ispeed);
	printf("ioctl TCGETS,opt.c_cflag = %#x\n", opt.c_cflag);

	speed_t change_speed = B9600;
	if(opt.c_ospeed == B9600)
		change_speed = B115200;

	//设置串口输出波特率
	cfsetospeed(&opt, change_speed);
	//设置串口输入波特率
	cfsetispeed(&opt, change_speed);
	//设置数据位数
	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |= CS8;
	//校验位
	opt.c_cflag &= ~PARENB;
	opt.c_iflag &= ~INPCK;
	//设置停止位
	opt.c_cflag &= ~CSTOPB;

	//更新配置
	// tcsetattr(fd, TCSANOW, &opt);   
	res = ioctl(fd,TCSETS, &opt);

	//再次读取
	res = ioctl(fd,TCGETS, &opt);

	opt.c_ispeed = opt.c_cflag & (CBAUD | CBAUDEX);
	opt.c_ospeed = opt.c_cflag & (CBAUD | CBAUDEX);

	printf("ioctl TCGETS after TCSETS\n");

	//输出读取到的值
  	printf("ioctl TCGETS,opt.c_ospeed = %#o\n", opt.c_ospeed);
	printf("ioctl TCGETS,opt.c_ispeed = %#o\n", opt.c_ispeed);
	printf("ioctl TCGETS,opt.c_cflag = %#x\n", opt.c_cflag);

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