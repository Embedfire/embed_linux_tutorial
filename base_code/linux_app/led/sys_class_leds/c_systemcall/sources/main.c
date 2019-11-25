#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

//ARM 开发板LED设备的路径
#define RLED_DEV_PATH "/sys/class/leds/red/brightness"
#define GLED_DEV_PATH "/sys/class/leds/green/brightness"
#define BLED_DEV_PATH "/sys/class/leds/blue/brightness"

//Ubuntu主机LED设备的路径，具体请根据自己的主机LED设备修改
// #define RLED_DEV_PATH "/sys/class/leds/input2::capslock/brightness"
// #define GLED_DEV_PATH "/sys/class/leds/input2::numlock/brightness"
// #define BLED_DEV_PATH "/sys/class/leds/input2::scrolllock/brightness"


int main(int argc, char *argv[])
{
	int res = 0;
	int r_fd, g_fd, b_fd;
	
	printf("This is the led demo\n");
	//获取红灯的设备文件描述符
	r_fd = open(RLED_DEV_PATH, O_WRONLY);
	if(r_fd < 0){
		printf("Fail to Open %s device\n", RLED_DEV_PATH);
		exit(1);
	}
	//获取绿灯的设备文件描述符
	g_fd = open(GLED_DEV_PATH, O_WRONLY);
	if(g_fd < 0){
		printf("Fail to Open %s device\n", GLED_DEV_PATH);
		exit(1);
	}
	//获取蓝灯的设备文件描述符
	b_fd = open(BLED_DEV_PATH, O_WRONLY);
	if(b_fd < 0){
		printf("Fail to Open %s device\n", BLED_DEV_PATH);
		exit(1);
	}

	while(1){
		//红灯亮
		write(r_fd, "255", 3);
		//延时1s
		sleep(1);
		//红灯灭
		write(r_fd, "0", 1);

		//绿灯亮
		write(g_fd, "255", 3);
		//延时1s
		sleep(1);
		//绿灯灭
		write(g_fd, "0", 1);

		//蓝灯亮
		write(b_fd, "255", 3);
		//延时1s
		sleep(1);
		//蓝灯亮
		write(b_fd, "0", 1);
	}
}