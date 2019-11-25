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

#include <stdio.h>
#include <string.h>

//要写入的字符串
const char buf[] = "filesystem_test:Hello World!\n";
//文件描述符
FILE *fp;
char str[100];


int main(void)
{
	//创建一个文件
	fp = fopen("filesystem_test.txt", "w+");
	//正常返回文件指针
	//异常返回NULL
	if(NULL == fp){
		printf("Fail to Open File\n");
		return 0;
	}
	//将buf的内容写入文件
	//每次写入1个字节，总长度由strlen给出
	fwrite(buf, 1, strlen(buf), fp);

	//写入Embedfire
	//每次写入1个字节，总长度由strlen给出
	fwrite("Embedfire\n", 1, strlen("Embedfire\n"),fp);

	//把缓冲区的数据立即写入文件 
	fflush(fp);

	//此时的文件位置指针位于文件的结尾处，使用fseek函数使文件指针回到文件头
	fseek(fp, 0, SEEK_SET);

	//从文件中读取内容到str中
	//每次读取100个字节，读取1次
	fread(str, 100, 1, fp);

	printf("File content:\n%s \n", str);

	fclose(fp);

	return 0;
}

