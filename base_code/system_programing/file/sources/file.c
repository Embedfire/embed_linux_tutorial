/* copy_file.c */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 1024 /* 每次读写缓存大小，影响运行效率*/

#define SRC_FILE_NAME "src_file" /* 源文件名 */

#define DEST_FILE_NAME "dest_file" /* 目标文件名文件名 */

#define OFFSET 	10240 /* 复制的数据大小 */


int main(void)
{
	int src_file, dest_file;	/* 文件描述符 */
	
	unsigned char buff[BUFFER_SIZE];
	
	int real_read_len;
	
	printf("open src_file with read only\n");
	/* 以只读方式打开源文件 */
	src_file = open(SRC_FILE_NAME, O_RDONLY);
	
	printf("open dest_file with write only\n");
	/* 以只写方式打开目标文件，若此文件不存在则创建该文件, 访问权限值为 644 */
	dest_file = open(DEST_FILE_NAME, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	
	if (src_file < 0 || dest_file < 0)
	{
		printf("Open file error\n");
		exit(1);
	}
	
	printf("move pointer to file end\n");
	/* 将源文件的读写指针移到最后 10KB 的起始位置*/
	lseek(src_file, -OFFSET, SEEK_END);
	
	/* 读取源文件的最后 10KB 数据并写到目标文件中，每次读写 1KB */
	while ((real_read_len = read(src_file, buff, sizeof(buff))) > 0)
	{
		printf("write data to dest_file\n");
		write(dest_file, buff, real_read_len);
	}
	
	/** 同步文件 */
	if(fsync(dest_file) == 0)
	{
		printf("wait all data write to the dest_file\n");
		close(dest_file);
		close(src_file);
	}

	/** 开始读取目标文件 */
	printf("start read dest_file\n");
	printf("*****************************************\n");

	dest_file = open(DEST_FILE_NAME, O_RDONLY);
	if(dest_file < 0)
	{
		printf("open dest_file fail\n");
		exit(1);
	}

	/* 将目标文件的读写指针移到开始 */
	lseek(dest_file, 0, SEEK_SET);

	/** 读取数据并且打印出来 */
	while ((real_read_len = read(src_file, buff, sizeof(buff))) > 0)
	{
		printf("%s",buff);
	}

	printf("*****************************************\n");

	return 0;
}