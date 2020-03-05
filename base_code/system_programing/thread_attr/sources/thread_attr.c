/** thread_attr.c */

/**
 * 函数原型 int pthread_attr_init(pthread_attr_t *attr)
 * 函数传入值 attr：线程属性结构指针
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 * 函数原型 int pthread_attr_setscope(pthread_attr_t *attr, int scope)
 * 函数传入值 attr：线程属性结构指针
 * cope
 *  PTHREAD_SCOPE_SYSTEM：绑定
 *  PTHREAD_SCOPE_PROCESS：非绑定
 * 函数返回值
 *  成功： 0
 *  出错： -1
 * 
 * 函数原型 int pthread_attr_setscope(pthread_attr_t *attr, int detachstate)
 * 函数传入值
 *  attr：线程属性
 *  detachstate:
 *   PTHREAD_CREATE_DETACHED：分离
 *   PTHREAD _CREATE_JOINABLE：非分离
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 * 函数原型 int pthread_attr_getschedparam (pthread_attr_t *attr, struct sched_param *param)
 * 函数传入值
 *  attr：线程属性结构指针
 *  param：线程优先级
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 * 
 * 函数原型 int pthread_attr_setschedparam (pthread_attr_t *attr, struct sched_param *param)
 * 函数传入值
 *  attr：线程属性结构指针
 *  param：线程优先级
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define REPEAT_NUMBER 3 /* 线程中的小任务数 */
#define DELAY_TIME_LEVELS 10.0 /* 小任务之间的最大时间间隔 */
int finish_flag = 0;

void *thrd_func(void *arg)
{
    int delay_time = 0;
    int count = 0;
    printf("Thread is starting\n");
    for (count = 0; count < REPEAT_NUMBER; count++)
    {
        delay_time = (int)(rand() * DELAY_TIME_LEVELS/(RAND_MAX)) + 1;
        sleep(delay_time);
        printf("\tThread : job %d delay = %d\n", count, delay_time);
    }

    printf("Thread finished\n");
    finish_flag = 1;
    pthread_exit(NULL);
}


int main(void)
{
    pthread_t thread;
    pthread_attr_t attr;
    int no = 0, res;
    void * thrd_ret;

    srand(time(NULL));

    /* 初始化线程属性对象 */
    res = pthread_attr_init(&attr);
    if (res != 0)
    {
        printf("Create attribute failed\n");
        exit(res);
    }

    /* 设置线程绑定属性 */
    res = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* 设置线程分离属性 */
    res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (res != 0)
    {
        printf("Setting attribute failed\n");
        exit(res);
    }

    res = pthread_create(&thread, &attr, thrd_func, NULL);
    if (res != 0)
    {
        printf("Create thread failed\n");
        exit(res);
    }

    /* 释放线程属性对象 */
    pthread_attr_destroy(&attr);

    printf("Create tread success\n");
    while(!finish_flag)
    {
        printf("Waiting for thread to finish...\n");
        sleep(1);
    }

    return 0;
}


