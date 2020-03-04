#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
 

//开发板上的KEY按键，请根据实际情况修改 
const char default_path[] = "/dev/input/by-path/platform-gpio-keys-event";

//开发板上的ON_OFF按键，请根据实际情况修改
//const char default_path[] = "/dev/input/by-path/platform-ci_hdrc.1-usb-0:1.1:1.0-event-mouse";


int main(int argc, char *argv[])
{
	int fd;
	struct input_event event;
	char *path;
	
	printf("This is a input device demo.\n");

	//若无输入参数则使用默认事件设备
	if(argc > 1)
		path = argv[1];
	else
		path = (char *)default_path;

	fd = open(path, O_RDONLY);
	if(fd < 0){
		printf("Fail to open device:%s.\n"
				"Please confirm the path or you have permission to do this.\n", path);
		exit(1);
	}	
	
	printf("Test device: %s.\nWaiting for input...\n", path);
	
	while(1){
		if(read(fd, &event, sizeof(event)) == sizeof(event)){
			
			//EV_SYN是事件分隔标志，不打印
			if(event.type != EV_SYN)
				printf("Event: time %ld.%ld, type %d, code %d,value %d\n",			
						event.time.tv_sec,event.time.tv_usec,
						event.type,
						event.code,
						event.value);
		}
	}
	close(fd);

	return 0;
}
