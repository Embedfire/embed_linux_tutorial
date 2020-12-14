/**
 * 线程按照其调度者可以分为用户级线程和核心级线程两种。
 * （1）用户级线程。
 * 用户级线程主要解决的是上下文切换的问题，它的调度算法和调度过程全部由用户自行选择决定，在运行
 * 时不需要特定的内核支持。在这里，操作系统往往会提供一个用户空间的线程库，该线程库提供了线程的
 * 创建、调度和撤销等功能，而内核仍然仅对进程进行管理。如果一个进程中的某一个线程调用了一个阻塞
 * 的系统调用函数，那么该进程包括该进程中的其他所有线程也同时被阻塞。这种用户级线程的主要缺点是
 * 在一个进程中的多个线程的调度中无法发挥多处理器的优势。
 * （2）轻量级进程。
 * 轻量级进程是内核支持的用户线程，是内核线程的一种抽象对象。每个线程拥有一个或多个轻量级线程，
 * 而每个轻量级线程分别被绑定在一个内核线程上。
 * （3）内核线程。
 * 这种线程允许不同进程中的线程按照同一相对优先调度方法进行调度，这样就可以发挥多处理器的并发优势。
 * 现在大多数系统都采用用户级线程与核心级线程并存的方法。一个用户级线程可以对应一个或几个核心级
 * 线程，也就是"一对一"或"多对一"模型。这样既可满足多处理机系统的需要，也可以最大限度地减少
 * 调度开销。
 * 使用线程机制大大加快上下文切换速度而且节省很多资源。但是因为在用户态和内核态均要实现调度管
 * 理，所以会增加实现的复杂度和引起优先级翻转的可能性。一个多线程程序的同步设计与调试也会增加程
 * 序实现的难度。
 */

/**
 * 函数原型 int pthread_create ((pthread_t *thread, pthread_attr_t *attr,void *(*start_routine)(void *), void *arg))
 * 函数传入值
 *  thread：线程标识符
 *  attr：线程属性设置，通常取为 NULL
 *  start_routine：线程函数的起始地址，是一个以指向 void 的指针作为参数和返回值的函数指针
 *  arg：传递给 start_routine 的参数
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 * 函数原型 void pthread_exit(void *retval)
 * 函数传入值 
 *  retval：线程结束时的返回值，可由其他函数如 pthread_join()来获取
 * 
 * 函数原型 int pthread_join ((pthread_t th, void **thread_return))
 * 函数传入值
 *  th：等待线程的标识符
 *  thread_return：用户定义的指针，用来存储被等待线程结束时的返回值（不为 NULL 时）
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 * 函数原型 int pthread_cancel((pthread_t th)
 * 函数传入值 th：要取消的线程的标识符
 * 函数返回值
 *  成功： 0
 *  出错：返回错误码
 * 
 */


#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/*要执行的线程*/
void *test_thread(void *arg)
{
    int num = (unsigned long long)arg; /** sizeof(void*) == 8 and sizeof(int) == 4 (64 bits) */

    printf("This is test thread, arg is %d\n", num);
    sleep(5);
    /*退出线程*/
    pthread_exit(NULL);
}


int main(void)
{
    pthread_t thread;
    void *thread_return;
    int arg = 520;
    int res;

    printf("start create thread\n");

    /*创建线程，线程为test_thread函数*/
    res = pthread_create(&thread, NULL, test_thread, (void*)(unsigned long long)(arg));
    if(res != 0)
    {
        printf("create thread fail\n");
        exit(res);
    }

    printf("create treads success\n");
    printf("waiting for threads to finish...\n");

    /*等待线程终止*/
    res = pthread_join(thread, &thread_return);
    if(res != 0)
    {
        printf("thread exit fail\n");
        exit(res);
    }

    printf("thread exit ok\n");

    return 0;
}


