#ifndef _SHM_DATA_H_
#define _SHM_DATA_H_
 
#define TEXT_SIZE 2048
 
struct shared_use_st
{
	int written;    //作为一个标志，非0：表示可读，0表示可写
	char text[TEXT_SIZE];//记录写入和读取的文本
};
 
#endif