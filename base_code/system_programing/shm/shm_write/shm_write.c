#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
 #include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "shm_data.h"
#include "../sem/sem.h"

int main()
{
	int running = 1;
	void *shm = NULL;
	struct shared_use_st *shared = NULL;
	char buffer[BUFSIZ + 1];//用于保存输入的文本
	int shmid;
	int semid;;//信号量标识符

	//创建共享内存
	shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0644|IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	//将共享内存连接到当前进程的地址空间
	shm = shmat(shmid, (void*)0, 0);
	if(shm == (void*)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	printf("Memory attached at %p\n", shm);

    semid = semget((key_t)6666, 1, 0666|IPC_CREAT); /* 创建一个信号量*/

	if(semid == -1)
	{
		printf("sem open fail\n");
		exit(EXIT_FAILURE); 
	}

	//设置共享内存
	shared = (struct shared_use_st*)shm;

	while(running)//向共享内存中写数据
	{
		//数据还没有被读取，则等待数据被读取,不能向共享内存中写入文本
		while(shared->written == 1)
		{
			sleep(1);
			printf("Waiting...\n");
		}
		//向共享内存中写入数据
		printf("Enter some text: ");
		fgets(buffer, BUFSIZ, stdin);
		strncpy(shared->text, buffer, TEXT_SIZE);

		//写完数据，设置written使共享内存段可读
		shared->written = 1;

		sem_v(semid);/* 释放信号量 */

		//输入了end，退出循环（程序）
		if(strncmp(buffer, "end", 3) == 0)
			running = 0;
	}

	//把共享内存从当前进程中分离
	if(shmdt(shm) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	sleep(2);
	exit(EXIT_SUCCESS);
}