/** thread_sem.c */

/**
 *  sem_init()用于创建一个信号量，并初始化它的值。
 *  sem_wait()和 sem_trywait()都相当于 P 操作，在信号量大于零时它们都能将信号量的值减一，两
 * 者的区别在于若信号量小于零时， sem_wait()将会阻塞进程，而 sem_trywait()则会立即返回。
 *  sem_post()相当于 V 操作，它将信号量的值加一同时发出信号来唤醒等待的进程。
 *  sem_getvalue()用于得到信号量的值。
 *  sem_destroy()用于删除信号量。
 * 
 * 函数原型 int sem_init(sem_t *sem,int pshared,unsigned int value)
 * 函数传入值
 *  sem：信号量指针
 *  pshared：决定信号量能否在几个进程间共享。由于目前 Linux 还没有实现进程间共享信号量，
 *  所以这个值只能够取 0，就表示这个信号量是当前进程的局部信号量
 *  value：信号量初始化值
 * 函数返回值
 *  成功： 0
 *  出错： -1 
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define THREAD_NUMBER 3 /* 线程数 */
#define REPEAT_NUMBER 4 /* 每个线程中的小任务数 */

sem_t sem[THREAD_NUMBER];

/*线程函数*/
void *thread_func(void *arg)
{
    int num = (unsigned long long)arg;
    int delay_time = 0;
    int count = 0;

    /* 等待信号量，进行 P 操作 */
    sem_wait(&sem[num]);

    printf("Thread %d is starting\n", num);
    for (count = 0; count < REPEAT_NUMBER; count++)
    {
        printf("\tThread %d: job %d \n",num, count);
        sleep(1);
    }

    printf("Thread %d finished\n", num);
    /*退出线程*/
    pthread_exit(NULL);
}



int main(void)
{
    pthread_t thread[THREAD_NUMBER];
    int i = 0, res;
    void * thread_ret;

    /*创建三个线程，三个信号量*/
    for (i = 0; i < THREAD_NUMBER; i++)
    {
        /*创建信号量，初始信号量值为0*/
        sem_init(&sem[i], 0, 0);
        /*创建线程*/
        res = pthread_create(&thread[i], NULL, thread_func, (void*)(unsigned long long)i);

        if (res != 0)
        {
            printf("Create thread %d failed\n", i);
            exit(res);
        }
    }

    printf("Create treads success\n Waiting for threads to finish...\n");

    /*按顺序释放信号量 V操作*/
    for (i = 0; i<THREAD_NUMBER ; i++)
    {
        /* 进行 V 操作 */
        sem_post(&sem[i]);
        /*等待线程执行完毕*/
        res = pthread_join(thread[i], &thread_ret);
        if (!res)
        {
            printf("Thread %d joined\n", i);
        }
        else
        {
            printf("Thread %d join failed\n", i);
        }

    }

    for (i = 0; i < THREAD_NUMBER; i++)
    {
        /* 删除信号量 */
        sem_destroy(&sem[i]);
    }

    return 0;
}
