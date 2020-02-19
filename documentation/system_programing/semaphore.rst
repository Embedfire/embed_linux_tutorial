本章先别发布，应该是在讲解线程后才发布
======================================

线程信号量
==========

这篇文章将讲述别一种线程间通信的机制——线程信号量，为了明确进程与线程信号量间的区别，本章
出现的信号量均为线程使用的信号量。注意请不要把它与之前所说的信号混淆了，信号与信号量是不同的两种机制。

线程信号量基本概念
------------------

信号量（Semaphore）是一种实现线程间通信的机制，可以实现线程之间同步或
临界资源的互斥访问，常用于协助一组相互竞争的线程来访问临界资源。在多线程
系统中，各线程之间需要同步或互斥实现临界资源的保护，信号量功能可以为用户提供这方面的支持。

在多线程操作系统环境下，多个线程会同时运行，并且一些线程之间可能存在一定
的关联。多个线程可能为了完成同一个线程会相互协作，这样形成线程之间的同步关系，因此可以使用信号量进行同步。

而且在不同线程之间，为了争夺有限的系统资源（硬件或软件资源）会进入竞争状
态，这就是线程之间的互斥关系。为了防止出现因多个程序同时访问一个共享资源而引发的一系列问题，我们
需要一种方法，它可以通过生成并使用令牌来授权，在任一时刻只能有一个执行线程访问代码的临界区域。临
界区域是指执行数据更新的代码需要独占式地执行。而信号量就可以提供这样的一种访问机制，让一个临界区
同一时间只有一个线程在访问它，因此信号量是可以用来调协线程对共享资源的访问的。线程之间的互斥与同
步关系存在的根源在于临界资源。临界资源是在同一个时刻只允许有限个（通常只有一个）线程可以访问（读）或
修改（写）的资源，通常包括硬件资源（处理器、内存、存储器以及其他外围设备等）和软件资源（共享代码段，共
享结构和变量等）。

抽象的来讲，信号量中存在一个非负整数，所有获取它的线程都会将该整数减
一（获取它当然是为了使用资源），当该整数值为零时，所有试图获取它的线程
都将处于阻塞状态。通常一个信号量的计数值用于对应有效的资源数，表示剩下的可
被占用的互斥资源数。其值的含义分两种情况：

-  0：表示没有可用的信号量，线程进入睡眠状态，直至信号量值大于 0。

-  正值：表示有一个或多个可用的信号量，线程可以使用该资源。线程将信号量值减
   1，表示它使用了一个资源单位。

对信号量的操作可以分为两个：

-  P
   操作：如果有可用的资源（信号量值大于0），则占用一个资源（给信号量
   值减去一，进入临界区代码）;如果没有可用的资源（信号量值等于
   0），则被阻塞到，直到系统将资源分配给该线程（进入等待队列，一直等到
   资源轮到该线程）。这就像你要把车开进停车场之前，先要向保安申请一张停车卡一样，P
   操作就是申请资源，如果申请成功，资源数（空闲的停车位）将会减少一个，如果申
   请失败，要不在门口等，要不就走人。

-  V
   操作：如果在该信号量的等待队列中有线程在等待资源，则唤醒一个阻塞的线程。如果
   没有线程等待它，则释放一个资源（给信号量值加一），就跟你从停车场出去的时候一样，空闲的停车位就会增加一个。

举个例子，就是两个线程共享信号量sem，sem可用信号量的数值为1，一旦其中一个线
程执行了P（sem）操作，它将得到信号量，并可以进入临界区，使sem减1。而第二个线程将被阻
止进入临界区，因为当它试图执行P（sem）操作时，sem为0，它会被挂起以等待第一个线程离开临界
区域并执行V（sem）操作释放了信号量，这时第二个线程就可以恢复执行。

线程信号量相关函数
------------------

如果要在Linux中使用信号量同步，需要包含头文件\ ``semaphore.h``\ 。

**主要用到的函数：**

.. code:: c

    int sem_init(sem_t *sem， int pshared， unsigned int value);
    int sem_destroy(sem_t *sem);
    int sem_wait(sem_t *sem);
    int sem_post(sem_t *sem);

-  sem\_init()：初始化信号量，其中sem是要初始化的信号量，pshared表示此信号量是在线
   程间共享还是线程间共享，由于目前
   Linux 还没有实现进程间共享信号量，所以这个值只能够取
   0，就表示这个信号量是当前进程的局部信号量，value是信号量的初始值。成功返回0，失败返回-1。

-  sem\_destroy()：销毁信号量，其中sem是要销毁的信号量。只有用sem\_init初
   始化的信号量才能用sem\_destroy()函数销毁。成功返回0，失败返回-1。

-  sem\_wait()：等待（获取）信号量，如果信号量的值大于0，将信号量的值减1，立即
   返回。如果信号量的值为0，则线程阻塞。相当于P操作。成功返回0，失败返回-1。

-  sem\_post();
   释放信号量，让信号量的值加1，相当于V操作。成功返回0，失败返回-1。

线程信号量的使用实例
--------------------

下面的例子是用信号量同步机制实现 3
个线程之间的有序执行，首先创建3个线程，然后在线程中首先使用sem\_wait()获取
信号量，然后随机睡眠3次，然后再通过sem\_post()函数释放信号量，再退出线程，这个
实验的目的是为了使用信号量在多线程间进行同步。

**代码如下：**

.. code:: c

    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <pthread.h>
    #include <semaphore.h>

    #define THREAD_NUMBER 3 /* 线程数 */
    #define REPEAT_NUMBER 3 /* 每个线程中的小任务数 */
    #define DELAY_TIME_LEVELS 3.0 /*小任务之间的最大时间间隔*/

    sem_t sem[THREAD_NUMBER];

    void *thread_func(void *arg)
    {
        int num = (unsigned long long)arg;
        int delay_time = 0;
        int count = 0;

        /* 进行 P 操作 */
        sem_wait(&sem[num]);

        printf("Thread %d is starting\n", num);
        for (count = 0; count < REPEAT_NUMBER; count++)
        {
            delay_time = (int)(rand() * DELAY_TIME_LEVELS/(RAND_MAX)) + 1;
            sleep(delay_time);
            printf("\tThread %d: job %d delay = %d\n",num, count, delay_time);
        }

        printf("Thread %d finished\n", num);
        pthread_exit(NULL);
    }



    int main(void)
    {
        pthread_t thread[THREAD_NUMBER];
        int no = 0, res;
        void * thread_ret;
        srand(time(NULL));

        for (no = 0; no < THREAD_NUMBER; no++)
        {
            sem_init(&sem[no], 0, 0);
            res = pthread_create(&thread[no], NULL, thread_func, (void*)(unsigned long long)no);

            if (res != 0)
            {
                printf("Create thread %d failed\n", no);
                exit(res);
            }
        }

        printf("Create treads success\n Waiting for threads to finish...\n");

        /* 对最后创建的线程的信号量进行 V 操作 */
        sem_post(&sem[THREAD_NUMBER - 1]);
        for (no = THREAD_NUMBER - 1; no >= 0; no--)
        {
            res = pthread_join(thread[no], &thread_ret);
            if (!res)
            {
                printf("Thread %d joined\n", no);
            }
            else
            {
                printf("Thread %d join failed\n", no);
            }

            /* 进行 V 操作 */
            sem_post(&sem[(no + THREAD_NUMBER - 1) % THREAD_NUMBER]);
        }

        for (no = 0; no < THREAD_NUMBER; no++)
        {
            /* 删除信号量 */
            sem_destroy(&sem[no]);
        }

        return 0;
    }

**实验结果如下：**

.. code:: bash

    ➜  thread_sem make     

    gcc -o thread_sem.o -c -g -Werror -I. -Iinclude -lpthread -static  thread_sem.c -g -MD -MF .thread_sem.o.d
    gcc -o targets thread_sem.o -g -Werror -I. -Iinclude -lpthread -static 


    ➜  thread_sem ./targets

    Create treads success
     Waiting for threads to finish...
    Thread 2 is starting
            Thread 2: job 0 delay = 1
            Thread 2: job 1 delay = 3
            Thread 2: job 2 delay = 3
    Thread 2 finished
    Thread 2 joined
    Thread 1 is starting
            Thread 1: job 0 delay = 3
            Thread 1: job 1 delay = 1
            Thread 1: job 2 delay = 1
    Thread 1 finished
    Thread 1 joined
    Thread 0 is starting
            Thread 0: job 0 delay = 3
            Thread 0: job 1 delay = 2
            Thread 0: job 2 delay = 1
    Thread 0 finished
    Thread 0 joined

