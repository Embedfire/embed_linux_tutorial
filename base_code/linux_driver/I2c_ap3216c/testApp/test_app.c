/*********************************************************************************
*Copyright(C),    embedfire
*FileName:        ap3216c
*Author:          lixiaojian
*Version:         v1.0
*Date:            20200710
*Description:     ap3216c
*Others:          //其他内容说明
*********************************************************************************/
#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
int main(int argc, char *argv[])
{
	int fd;
	char *filename;
	unsigned int databuf[3];
	unsigned int ir, als, ps;
	int ret = 0;

	if (argc != 1) {
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = "/dev/ap3216c";
	fd = open(filename, O_RDWR);
	if(fd < 0) {
		printf("can't open file %s\r\n", filename);
		return -1;
	}

	while (1) {
		ret = read(fd, databuf, sizeof(databuf));
		if(ret == 0) { 			/* 数据读取成功 */
			als =  databuf[0]; 	/* ambient 传感器数据 */
			ir  = databuf[1]; 	/* infrared ray 传感器数据 */
			ps  =  databuf[2]; 	/* proximity 传感器数据 */
			printf("Ambient : [%4d], Infrared ray : [%4d], Proximity : [%4d]\r\n", als, ir, ps);
		}
		usleep(300000); /*300ms */
	}
	close(fd);	/* 关闭文件 */	
	return 0;
}

