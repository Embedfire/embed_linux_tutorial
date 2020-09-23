#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

 
/* 程序的入口函数 */
int main(int argc, char *argv[])
{
	int fd;
	unsigned char buf[6];	/* 定义存放数据的数组 */
	int length;

	/* 以只读方式打开设备节点 */
	fd = open("/dev/infra", O_RDONLY);
	if(fd == -1)
	{
		printf("open failed!\n");
		return -1;
	}

	while(1)
	{
		length = read(fd, buf, 6);	/* 读取温湿度数据 */
		if(length == -1)
		{
			printf("read error!\n");
			return -1;
		}
		/* 将数据从终端打印出来 */
		printf("Temp : %d, Humi : %d\n", buf[2], buf[0]);
		sleep(1);
	}

 
	/* 关闭DHT11设备节点 */
	close(fd);
	return 0;
}


   