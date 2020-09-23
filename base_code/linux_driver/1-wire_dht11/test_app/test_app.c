#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

 

int main(int argc, char *argv[])
{
	int fd;
	unsigned char buf[6];	
	int length;


	fd = open("/dev/dht11", O_RDONLY);
	if(fd == -1)
	{
		printf("open failed!\n");
		return -1;
	}

	while(1)
	{
		length = read(fd, buf, 6);	
		if(length == -1) {
			printf("read error!\n");
			return -1;
		}
		/* 打印 */
		printf("Temperature : %d, Humi : %d\n", buf[2], buf[0]);
		sleep(1);
	}

 
	/* 关闭DHT11设备节点 */
	close(fd);
	return 0;
}


   