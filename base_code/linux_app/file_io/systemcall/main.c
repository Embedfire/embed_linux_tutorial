/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   文件系统测试代码（使用系统调用）
  ******************************************************************
  * @attention
  *
  * 实验平台: 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************
  */  
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

//文件描述符
int fd;
char str[100];


int main(void)
{
	//创建一个文件
	fd = open("testscript.sh", O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
	//文件描述符fd为非负整数
	if(fd < 0){
		printf("Fail to Open File\n");
		return 0;
	}
	//写入字符串pwd
	write(fd, "pwd\n", strlen("pwd\n"));

	//写入字符串ls
	write(fd, "ls\n", strlen("ls\n"));

	//此时的文件指针位于文件的结尾处，使用lseek函数使文件指针回到文件头
	lseek(fd, 0, SEEK_SET);

	//从文件中读取100个字节的内容到str中，该函数会返回实际读到的字节数
	read(fd, str, 100);

	printf("File content:\n%s \n", str);

	close(fd);

	return 0;
}

