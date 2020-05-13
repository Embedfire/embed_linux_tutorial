POSIX信号量
===========

这篇文章将讲述别一种进程/线程间通信的机制——POSIX信号量，为了明确与system
V信号量间的区别，若非特别说明，本章出现的信号量均为POSIX信号量。也要注意请不要把它与之前所说的信号混淆了，信号与信号量是不同的两种机制。

POSIX信号量基本概念
-------------------

信号量（Semaphore）是一种实现进程/线程间通信的机制，可以实现进程/线程之间同步或临界资源的互斥访问，常用于协助一组相互竞争的进程/线程来访问临界资源。在多进程/线程系统中，各进程/线程之间需要同步或互斥实现临界资源的保护，信号量功能可以为用户提供这方面的支持。

在 POSIX
标准中，信号量分两种，一种是无名信号量，一种是有名信号量。无名信号量一般用于进程/线程间同步或互斥，而有名信号量一般用于进程间同步或互斥。有名信号量和无名信号量的差异在于创建和销毁的形式上，但是其他工作一样，无名信号量则直接保存在内存中，而有名信号量要求创建一个文件。

正如其名，无名信号量没有名字，它只能存在于内存中，这就要求使用信号量的进程/线程必须能访问无名信号量所在的这一块内存，所以无名信号量只能应用在同一进程内的线程之间同步或者互斥。相反，有名信号量可以通过名字访问，因此可以被任何知道它们名字的进程或者进程/线程使用。单个进程中使用
POSIX 信号量时，无名信号量更简单，多个进程间使用 POSIX
信号量时，有名信号量更简单。

在多进程/多进程/线程操作系统环境下，多个进程/线程会同时运行，并且一些进程/线程之间可能存在一定的关联。多个进程/线程可能为了完成同一个进程/线程会相互协作，这样形成进程/线程之间的同步关系，因此可以使用信号量进行同步。

而且在不同进程/线程之间，为了争夺有限的系统资源（硬件或软件资源）会进入竞争状态，这就是进程/线程之间的互斥关系。为了防止出现因多个程序同时访问一个共享资源而引发的一系列问题，我们需要一种方法，它可以通过生成并使用令牌来授权，在任一时刻只能有一个执行进程/线程访问代码的临界区域。临界区域是指执行数据更新的代码需要独占式地执行。而信号量就可以提供这样的一种访问机制，让一个临界区同一时间只有一个进程/线程在访问它，因此信号量是可以用来调协进程/线程对共享资源的访问的。进程/线程之间的互斥与同步关系存在的根源在于临界资源。临界资源是在同一个时刻只允许有限个（通常只有一个）进程/线程可以访问（读）或修改（写）的资源，通常包括硬件资源（处理器、内存、存储器以及其他外围设备等）和软件资源（共享代码段，共享结构和变量等）。

抽象的来讲，信号量中存在一个非负整数，所有获取它的进程/线程都会将该整数减一（获取它当然是为了使用资源），当该整数值为零时，所有试图获取它的进程/线程都将处于阻塞状态。通常一个信号量的计数值用于对应有效的资源数，表示剩下的可被占用的互斥资源数。其值的含义分两种情况：

-  0：表示没有可用的信号量，进程/线程进入睡眠状态，直至信号量值大于 0。

-  正值：表示有一个或多个可用的信号量，进程/线程可以使用该资源。进程/线程将信号量值减
   1，表示它使用了一个资源单位。

对信号量的操作可以分为两个：

-  P
   操作：如果有可用的资源（信号量值大于0），则占用一个资源（给信号量值减去一，进入临界区代码）;如果没有可用的资源（信号量值等于
   0），则被阻塞到，直到系统将资源分配给该进程/线程（进入等待队列，一直等到资源轮到该进程/线程）。这就像你要把车开进停车场之前，先要向保安申请一张停车卡一样，P
   操作就是申请资源，如果申请成功，资源数（空闲的停车位）将会减少一个，如果申请失败，要不在门口等，要不就走人。

-  V
   操作：如果在该信号量的等待队列中有进程/线程在等待资源，则唤醒一个阻塞的进程/线程。如果没有进程/线程等待它，则释放一个资源（给信号量值加一），就跟你从停车场出去的时候一样，空闲的停车位就会增加一个。

举个例子，就是两个进程/线程共享信号量sem，sem可用信号量的数值为1，一旦其中一个进程/线程执行了P（sem）操作，它将得到信号量，并可以进入临界区，使sem减1。而第二个进程/线程将被阻止进入临界区，因为当它试图执行P（sem）操作时，sem为0，它会被挂起以等待第一个进程/线程离开临界区域并执行V（sem）操作释放了信号量，这时第二个进程/线程就可以恢复执行。

POSIX有名信号量
---------------

如果要在Linux中使用信号量同步，需要包含头文件\ ``semaphore.h``\ 。

有名信号量其实是一个文件，它的名字由类似\ ``"sem.[信号量名字]"``\ 这样的字符串组成，注意看文件名前面有\ ``"sem."``\ ，它是一个特殊的信号量文件，在创建成功之后，系统会将其放置在\ ``/dev/shm``\ 路径下，不同的进程间只要约定好一个相同的信号量文件名字，就可以访问到对应的有名信号量，并且借助信号量来进行同步或者互斥操作，需要注意的是，有名信号量是一个文件，在进程退出之后它们并不会自动消失，而需要手工删除并释放资源。

**主要用到的函数：**

.. code:: c

        sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
        int sem_wait(sem_t *sem);
        int sem_trywait(sem_t *sem);
        int sem_post(sem_t *sem);
        int sem_close(sem_t *sem);
        int sem_unlink(const char *name);

sem_open()函数用于打开/创建一个有名信号量

**参数：**

-  name：打开或者创建信号量的名字。
-  oflag：当指定的文件不存在时，可以指定 O_CREATE 或者 O_EXEL
   进行创建操作，如果指定为0，后两个参数可省略，否则后面两个参数需要带上。
-  mode：八进制的文件读写权限。
-  value：信号量初始的值，这这个参数只有在新创建的时候才需要设置，如果是打开已有的，不需要在去指定，否则会报错。该值不允许大于系统指定值SEM_VALUE_MAX。
-  sem_open()函数的返回值是一个sem_t类型的指针，它指向已经创建/打开的信号量，后续的函数都通过改信号量指针去访问对应的信号量。

sem_wait()函数是等待（获取）信号量，如果信号量的值大于0，将信号量的值减1，立即返回。如果信号量的值为0，则进程/线程阻塞。相当于P操作。成功返回0，失败返回-1。

sem_trywait()函数也是等待信号量，如果指定信号量的计数器为0，那么直接返回EAGAIN错误，而不是阻塞等待。

sem_post()函数是释放信号量，让信号量的值加1，相当于V操作。成功返回0，失败返回-1。

sem_close()函数用于关闭一个信号量，这表示当前进程/线程取消对信号量的使用，它的作用仅在当前进程/线程，其他进程/线程依然可以使用该信号量，同时当进程结束的时候，无论是正常退出还是信号中断退出的进程，内核都会主动调用该函数去关闭进程使用的信号量，即使从此以后都没有其他进程/线程在使用这个信号量了，内核也会维持这个信号量。

sem_unlink()函数就是主动删除一个信号量，直接删除指定名字的信号量文件。

POSIX无名信号量
---------------

无名信号量的操作与有名信号量差不多的，但它不使用文件系统标识，直接存在程序运行的内存中，不同进程之间不能访问，不能用于不同进程之间相互访问。同样的一个父进程初始化一个信号量，然后fork其副本得到的是该信号量的副本，这两个信号量之间并不存在关系。

**主要用到的函数：**

.. code:: c

        int sem_init(sem_t *sem， int pshared， unsigned int value);
        int sem_destroy(sem_t *sem);
        int sem_wait(sem_t *sem);
        int sem_trywait(sem_t *sem);
        int sem_post(sem_t *sem);

-  sem_init()：初始化信号量，其中sem是要初始化的信号量，pshared表示此信号量是在进程间共享还是线程间共享，由于目前
   Linux 还没有实现进程间共享信号量，所以这个值只能够取
   0，就表示这个信号量是当前进程的局部信号量，value是信号量的初始值。成功返回0，失败返回-1。

-  sem_destroy()：销毁信号量，其中sem是要销毁的信号量。只有用sem_init初始化的信号量才能用sem_destroy()函数销毁。成功返回0，失败返回-1。

sem_wait()、sem_trywait()、sem_post()等函数与有名信号量的使用是一样的。

POSIX信号量的使用实例
---------------------

有名信号量
~~~~~~~~~~

有名信号量中创建了两个进程，然后进程之间通有名信号量进行同步操作。

有名信号量的使用操作步骤如下： 

1.通过sem_open()函数打开或者创建一个信号量。 

2.调用sem_wait()函数获取到信号量，然后打印信息到终端。 

3.使用sem_post()函数释放信号量。 

4.使用完毕后将信号量关闭并且删除：sem_close()、sem_unlink()。

.. code:: c

    #include <unistd.h>
    #include <stdio.h>
    #include <pthread.h>
    #include <semaphore.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <fcntl.h>
    #include <sys/wait.h>

    int main(int argc, char **argv)
    {
        int pid;
        sem_t *sem;
        const char sem_name[] = "my_sem_test";

        pid = fork();

        if (pid < 0) {
            printf("error in the fork!\n");
        } else if (pid == 0) {
            
            sem = sem_open(sem_name, O_CREAT, 0644, 1);

            if (sem == SEM_FAILED) {
                printf("unable to create semaphore...\n");

                sem_unlink(sem_name);

                exit(-1);
            }

            sem_wait(sem);

            for (int i = 0; i < 3; ++i) {

                printf("childe process run: %d\n", i);
                sleep(1);
            }

        sem_post(sem);

        } else {

            sem = sem_open(sem_name, O_CREAT, 0644, 1);
            
            if (sem == SEM_FAILED) {
                printf("unable to create semaphore...\n");

                sem_unlink(sem_name);

                exit(-1);
            }

            sem_wait(sem);

            for (int i = 0; i < 3; ++i) {

                printf("parent process run: %d\n", i);
                sleep(1);
            }

            sem_post(sem);

            wait(NULL);
        }

        sem_close(sem);
        sem_unlink(sem_name);

        return 0;
    }

编译并且运行后，实验现象如下：

.. code:: bash

     parent process run: 0
     parent process run: 1
     parent process run: 2
     childe process run: 0
     childe process run: 1
     childe process run: 2

在代码的运行过程中，如果你打开一个新的终端，并且输入以下命令：

.. code:: bash

    ls -l /dev/shm

    -rw-r--r--  1 root  root    32 2月  14 13:31 sem.my_sem_test

那么你可以发现在\ ``/dev/shm``\ 目录下存在一个\ ``sem.my_sem_test``\ 文件，这就是我们实验中创建的一个信号量，当进程运行完毕，这个信号量将会被删除。

无名信号量
~~~~~~~~~~

下面的例子是用无名信号量同步机制实现 3
个线程之间的有序执行，首先创建3个线程，然后在线程中首先使用sem_wait()获取信号量，然后随机睡眠3次，然后再通过sem_post()函数释放信号量，再退出进线程，这个实验的目的是为了使用信号量在多线程间进行同步。

**代码如下：**

.. code:: c

    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <pthread.h>
    #include <semaphore.h>

    #define THREAD_NUMBER 3 /* 进程/线程数 */
    #define REPEAT_NUMBER 3 /* 每个进程/线程中的小任务数 */
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

        /* 对最后创建的进程/线程的信号量进行 V 操作 */
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

