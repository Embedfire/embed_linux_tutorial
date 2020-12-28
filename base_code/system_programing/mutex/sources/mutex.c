/** mutex */

/**
 * 互斥锁是用一种简单的加锁方法来控制对共享资源的原子操作。这个互斥锁只有两种状态，也就是上锁和解锁，
 * 可以把互斥锁看作某种意义上的全局变量。在同一时刻只能有一个线程掌握某个互斥锁，拥有上锁状态的线程能
 * 够对共享资源进行操作。 若其他线程希望上锁一个已经被上锁的互斥锁， 则该线程就会挂起，直到上锁的线程
 * 释放掉互斥锁为止。 可以说， 这把互斥锁保证让每个线程对共享资源按顺序进行原子操作。
 *  互斥锁初始化： pthread_mutex_init()
 *  互斥锁上锁： pthread_mutex_lock()
 *  互斥锁判断上锁： pthread_mutex_trylock()
 *  互斥锁接锁： pthread_mutex_unlock()
 *  消除互斥锁： pthread_mutex_destroy()
 * 
 * 函数原型 int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
 * 函数传入值 mutex：互斥锁
 * Mutexattr：
 *  PTHREAD_MUTEX_INITIALIZER：创建快速互斥锁
 *  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP：创建递归互斥锁
 *  PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP： 创建检错互斥锁
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 * 函数原型
 * int pthread_mutex_lock(pthread_mutex_t *mutex,)
 * int pthread_mutex_trylock(pthread_mutex_t *mutex,)
 * int pthread_mutex_unlock(pthread_mutex_t *mutex,)
 * int pthread_mutex_destroy(pthread_mutex_t *mutex,)
 * 函数传入值 mutex：互斥锁
 * 函数返回值
 *  成功： 0
 *  出错：-1
 * 
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_NUMBER 3 /* 线程数 */

pthread_mutex_t mutex;

void *thread_func(void *arg)
{
    int num = (unsigned long long)arg; /** sizeof(void*) == 8 and sizeof(int) == 4 (64 bits) */
    int sleep_time = 0;
    int res;

    /* 互斥锁上锁 */
    res = pthread_mutex_lock(&mutex);
    if (res)
    {   /*获取失败*/
        printf("Thread %d lock failed\n", num);

        /* 互斥锁解锁 */
        pthread_mutex_unlock(&mutex);

        pthread_exit(NULL);
    }

    printf("Thread %d is hold mutex\n", num);

    /*睡眠一定时间*/
    sleep(2);

    printf("Thread %d freed mutex\n\n", num);

    /* 互斥锁解锁 */
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}


int main(void)
{
    pthread_t thread[THREAD_NUMBER];
    int num = 0, res;

    srand(time(NULL));

    /* 互斥锁初始化 */
    pthread_mutex_init(&mutex, NULL);
    for (num = 0; num < THREAD_NUMBER; num++)
    {
        /*创建线程*/
        res = pthread_create(&thread[num], NULL, thread_func, (void*)(unsigned long long)num);
        if (res != 0)
        {
            printf("Create thread %d failed\n", num);
            exit(res);
        }
    }

    for (num = 0; num < THREAD_NUMBER; num++)
    {    
        /*等待线程结束*/
        pthread_join(thread[num], NULL);
    }

    /*销毁互斥锁*/
    pthread_mutex_destroy(&mutex);
    
    return 0;
}


