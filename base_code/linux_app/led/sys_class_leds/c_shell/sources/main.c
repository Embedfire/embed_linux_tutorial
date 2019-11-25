#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


//通过system执行shell命令
//ARM 开发板LED设备的路径
#define RLED_ON	    system("echo 255 > /sys/class/leds/red/brightness");
#define RLED_OFF    system("echo 0 > /sys/class/leds/red/brightness");
#define GLED_ON	    system("echo 255 > /sys/class/leds/green/brightness");
#define GLED_OFF    system("echo 0 > /sys/class/leds/green/brightness");
#define BLED_ON	    system("echo 255 > /sys/class/leds/blue/brightness");
#define BLED_OFF    system("echo 0 > /sys/class/leds/blue/brightness");

//通过system执行shell命令
//Ubuntu主机LED设备的路径，具体请根据自己的主机LED设备修改
// #define RLED_ON	    system("echo 255 > /sys/class/leds/input2::capslock/brightness");
// #define RLED_OFF    system("echo 0 > /sys/class/leds/input2::capslock/brightness");
// #define GLED_ON	    system("echo 255 > /sys/class/leds/input2::numlock/brightness");
// #define GLED_OFF    system("echo 0 > /sys/class/leds/input2::numlock/brightness");
// #define BLED_ON	    system("echo 255 > /sys/class/leds/input2::scrolllock/brightness");
// #define BLED_OFF    system("echo 0 > /sys/class/leds/input2::scrolllock/brightness");

int main(int argc, char *argv[])
{
	while(1){
		printf("This is the led demo.\n");
		RLED_ON;
		sleep(1);
		RLED_OFF

		GLED_ON
		sleep(1);
		GLED_OFF

		BLED_ON
		sleep(1);
		BLED_OFF
	}
}