POSIX信号量
===========

本章节将讲述另一种进程/线程间通信的机制——POSIX信号量，为了明确与systemV信号量间的区别，
若非特别说明，本章出现的信号量均为POSIX信号量。也要注意请不要把它与之前所说的信号混淆了，
信号与信号量是不同的两种机制。

POSIX信号量基本概念
-------------------

信号量（Semaphore）是一种实现进程/线程间通信的机制，可以实现进程/线程之间同步或临界资源的互斥访问，
常用于协助一组相互竞争的进程/线程来访问临界资源。在多进程/线程系统中，
各进程/线程之间需要同步或互斥实现临界资源的保护，信号量功能可以为用户提供这方面的支持。

在 POSIX标准中，信号量分两种，一种是无名信号量，一种是有名信号量。
无名信号量一般用于进程/线程间同步或互斥，而有名信号量一般用于进程间同步或互斥。
有名信号量和无名信号量的差异在于创建和销毁的形式上，但是其他工作一样，无名信号量则直接保存在内存中，
而有名信号量则要求创建一个文件。

正如其名，无名信号量没有名字，它只能存在于内存中，这就要求使用信号量的进程/线程必须能访问无名信号量所在的这一块内存，
所以无名信号量只能应用在同一进程内的线程之间同步或者互斥。相反，有名信号量可以通过名字访问，
因此可以被任何知道它们名字的进程或者进程/线程使用。单个进程中使用POSIX 信号量时，无名信号量更简单，
多个进程间使用 POSIX信号量时，有名信号量更简单。

在多进程/多进程/线程操作系统环境下，多个进程/线程会同时运行，并且一些进程/线程之间可能存在一定的关联。
多个进程/线程可能为了完成同一个进程/线程会相互协作，这样形成进程/线程之间的同步关系，因此可以使用信号量进行同步。

而且在不同进程/线程之间，为了争夺有限的系统资源（硬件或软件资源）会进入竞争状态，这就是进程/线程之间的互斥关系。
为了防止出现因多个程序同时访问一个共享资源而引发的一系列问题，我们需要一种方法，它可以通过生成并使用令牌来授权，
在任一时刻只能有一个执行进程/线程访问代码的临界区域。

临界区域是指执行数据更新的代码需要独占式地执行。
而信号量就可以提供这样的一种访问机制，让一个临界区同一时间只有一个进程/线程在访问它，
因此信号量是可以用来调协进程/线程对共享资源的访问的。进程/线程之间的互斥与同步关系存在的根源在于临界资源。
临界资源是在同一个时刻只允许有限个（通常只有一个）进程/线程可以访问（读）或修改（写）的资源，
通常包括硬件资源（处理器、内存、存储器以及其他外围设备等）和软件资源（共享代码段，共享结构和变量等）。

抽象的来讲，信号量中存在一个非负整数，所有获取它的进程/线程都会将该整数减一（获取它当然是为了使用资源），
当该整数值为零时，所有试图获取它的进程/线程都将处于阻塞状态。通常一个信号量的计数值用于对应有效的资源数，
表示剩下的可被占用的互斥资源数。其值的含义分两种情况：

-   0：表示没有可用的信号量，进程/线程进入睡眠状态，直至信号量值大于 0。

-   正值：表示有一个或多个可用的信号量，进程/线程可以使用该资源。进程/线程将信号量值减1，
    表示它使用了一个资源单位。

对信号量的操作可以分为两个：

-   P 操作：如果有可用的资源（信号量值大于0），则占用一个资源（给信号量值减去一，进入临界区代码）;
    如果没有可用的资源（信号量值等于0），则被阻塞到，直到系统将资源分配给该进程/线程（进入等待队列，
    一直等到资源轮到该进程/线程）。这就像你要把车开进停车场之前，先要向保安申请一张停车卡一样，
    P操作就是申请资源，如果申请成功，资源数（空闲的停车位）将会减少一个，如果申请失败，要不在门口等，要不就走人。

-   V 操作：如果在该信号量的等待队列中有进程/线程在等待资源，则唤醒一个阻塞的进程/线程。如果没有进程/线程等待它，
    则释放一个资源（给信号量值加一），就跟你从停车场出去的时候一样，空闲的停车位就会增加一个。

举个例子，就是两个进程/线程共享信号量sem，sem可用信号量的数值为1，一旦其中一个进程/线程执行了P（sem）操作，
它将得到信号量，并可以进入临界区，使sem减1。而第二个进程/线程将被阻止进入临界区，因为当它试图执行P（sem）操作时，
sem为0，它会被挂起以等待第一个进程/线程离开临界区域并执行V（sem）操作释放了信号量，这时第二个进程/线程就可以恢复执行。

POSIX有名信号量
---------------

如果要在Linux中使用信号量同步，需要包含头文件 ``semaphore.h`` 。

有名信号量其实是一个文件，它的名字由类似 ``“sem.[信号量名字]”`` 这样的字符串组成，注意看文件名前面有 ``“sem.”`` ，
它是一个特殊的信号量文件，在创建成功之后，系统会将其放置在 ``/dev/shm`` 路径下，
不同的进程间只要约定好一个相同的信号量文件名字，就可以访问到对应的有名信号量，
并且借助信号量来进行同步或者互斥操作，需要注意的是，有名信号量是一个文件，在进程退出之后它们并不会自动消失，
而需要手工删除并释放资源。

主要用到的函数：

.. code:: c

        sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
        int sem_wait(sem_t *sem);
        int sem_trywait(sem_t *sem);
        int sem_post(sem_t *sem);
        int sem_close(sem_t *sem);
        int sem_unlink(const char *name);

-   sem_open()函数用于打开/创建一个有名信号量，它的参数说明如下：

    -   name：打开或者创建信号量的名字。
    -   oflag：当指定的文件不存在时，可以指定 O_CREATE 或者 O_EXEL进行创建操作，
        如果指定为0，后两个参数可省略，否则后面两个参数需要带上。
    -   mode：数字表示的文件读写权限，如果信号量已经存在，本参数会被忽略。
    -   value：信号量初始的值，这这个参数只有在新创建的时候才需要设置，如果信号量已经存在，本参数会被忽略。
    -   返回值：返回值是一个sem_t类型的指针，它指向已经创建/打开的信号量，
        后续的函数都通过改信号量指针去访问对应的信号量。

-   sem_wait()函数是等待（获取）信号量，如果信号量的值大于0，将信号量的值减1，立即返回。如果信号量的值为0，
    则进程/线程阻塞。相当于P操作。成功返回0，失败返回-1。

-   sem_trywait()函数也是等待信号量，如果指定信号量的计数器为0，那么直接返回EAGAIN错误，而不是阻塞等待。

-   sem_post()函数是释放信号量，让信号量的值加1，相当于V操作。成功返回0，失败返回-1。

-   sem_close()函数用于关闭一个信号量，这表示当前进程/线程取消对信号量的使用，它的作用仅在当前进程/线程，
    其他进程/线程依然可以使用该信号量，同时当进程结束的时候，无论是正常退出还是信号中断退出的进程，
    内核都会主动调用该函数去关闭进程使用的信号量，即使从此以后都没有其他进程/线程在使用这个信号量了，
    内核也会维持这个信号量。

-   sem_unlink()函数就是主动删除一个信号量，直接删除指定名字的信号量文件。

POSIX无名信号量
---------------

无名信号量的操作与有名信号量差不多，但它不使用文件系统标识，直接存在程序运行的内存中，
不同进程之间不能访问，不能用于不同进程之间相互访问。同样的一个父进程初始化一个信号量，
然后fork其副本得到的是该信号量的副本，这两个信号量之间并不存在关系。

主要用到的函数：

.. code:: c

        int sem_init(sem_t *sem， int pshared， unsigned int value);
        int sem_destroy(sem_t *sem);
        int sem_wait(sem_t *sem);
        int sem_trywait(sem_t *sem);
        int sem_post(sem_t *sem);

-   sem_init()：初始化信号量。

    -   其中sem是要初始化的信号量，不要对已初始化的信号量再做sem_init操作，会发生不可预知的问题。
    -   pshared表示此信号量是在进程间共享还是线程间共享，由于目前Linux 还没有实现进程间共享无名信号量，
        所以这个值只能够取0，表示这个信号量是当前进程的局部信号量。
    -   value是信号量的初始值。
    -   返回值：成功返回0，失败返回-1。

-   sem_destroy()：销毁信号量，其中sem是要销毁的信号量。只有用sem_init初始化的信号量才能用sem_destroy()函数销毁。
    成功返回0，失败返回-1。

-   sem_wait()、sem_trywait()、sem_post()等函数与有名信号量的使用是一样的。

POSIX信号量的使用示例
---------------------

有名信号量
~~~~~~~~~~

首先来分析有名信号量的示例代码：

.. code-block:: c
    :caption: POSIX有名信号量（base_code/system_programing/posix_sem1/sources/posix_sem.c文件）
    :emphasize-lines: 23,25,35,41,45,49,52,62,68,72
    :linenos:

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
        } 
        /* 子进程 */
        else if (pid == 0) {
            /*创建/打开一个初始值为1的信号量*/
            sem = sem_open(sem_name, O_CREAT, 0644, 1);

            if (sem == SEM_FAILED) {
                printf("unable to create semaphore...\n");

                sem_unlink(sem_name);

                exit(-1);
            }
            /*获取信号量*/
            sem_wait(sem);

            for (int i = 0; i < 3; ++i) {

                printf("child process run: %d\n", i);
                /*睡眠释放CPU占用*/
                sleep(1);
            }

        /*释放信号量*/
        sem_post(sem);

        }
        /* 父进程 */
        else {

            /*创建/打开一个初始值为1的信号量*/
            sem = sem_open(sem_name, O_CREAT, 0644, 1);
            
            if (sem == SEM_FAILED) {
                printf("unable to create semaphore...\n");

                sem_unlink(sem_name);

                exit(-1);
            }
            /*申请信号量*/
            sem_wait(sem);

            for (int i = 0; i < 3; ++i) {

                printf("parent process run: %d\n", i);
                /*睡眠释放CPU占用*/
                sleep(1);
            }

            /*释放信号量*/
            sem_post(sem);
            /*等待子进程结束*/
            wait(NULL);

            /*关闭信号量*/
            sem_close(sem);
            /*删除信号量*/
            sem_unlink(sem_name);
        }

        return 0;
    }

本代码示例的分析如下： 

-   第23~47行，前面通过fork创建了子进程，这部分是子进程的代码。

    -   第25行，通过sem_open()函数打开或者创建了一个信号量，信号量的初始值为1。
    -   第35行，调用sem_wait()尝试获取信号量，若信号量值为0，代码将阻塞在此处等待。
    -   第41行，这是在for循环里的一个小睡眠，循环里每打印一句之后都释放CPU一段时间。以便其它进程运行。
    -   第45行，循环执行完毕，调用sem_post()释放信号量。

-   第49~80行，这部分是父进程的代码。

    -   第52~72行，它与子进程的内容完全一致。都是打开、获取信号量后循环打印，然后释放信号量。
    -   第74~79行，父进程内等待子进程结束后调用sem_close()和sem_unlink()关闭和释放信号量。

本代码两个进程for循环的sleep()是特意加进去模拟释放CPU操作的，进程A释放CPU后，按通常情况来说，
其它进程B会获得CPU而执行代码。但由于本示例的父子进程打印操作时都需要等待同一个信号量，所以进程A虽然睡眠，
但由于还没有释放CPU，进程B由于得不到信号量，并不会执行。

因此，我们可以推算得到这样的结果：

-   示例代码由于信号量的控制，运行后得到的结果是：进程A连续打印0，1，2三条语句，
    而进程B在A释放信号量后，B连续打印0，1，2三条语句。

-   ``假如注释掉示例代码所有跟信号量相关的操作（保留for循环里的sleep）`` ，那么由于sleep的存在，
    运行后得到的结果是：进程A打印0后进入睡眠释放CPU，进程B打印0后进入睡眠释放CPU；进程A打印1、进程B打印1...
    即这两个进程轮流执行，轮流打印。



实验操作
^^^^^^^^^^^^^^^^^^

本实验的代码存储在 ``base_code/system_programing/posix_sem1`` 目录中，编译及运行过程如下：

.. code:: bash

    # 以下操作在 base_code/system_programing/posix_sem1 代码目录进行
    # 编译X86版本程序
    make
    # 运行X86版本程序
    ./build_x86/posix_sem1_demo 
    
    # 以下是运行的输出
    parent process run: 0
    parent process run: 1
    parent process run: 2
    child process run: 0
    child process run: 1
    child process run: 2

可以看到，两个进程是分别连续打印的。感兴趣的话可以尝试把父、子进程的等待信号量操作sem_wait都注释掉，观看实验现象。

注：由于fork()后先执行父进程还是子进程是说不定的，只要能区分出是连续打印还是轮流打印即可看出信号量在本示例中的作用。



在代码的运行过程中，如果你打开一个新的终端，并且输入以下命令：

.. code:: bash

    ls -l /dev/shm

    -rw-r--r--  1 root  root    32 2月  14 13:31 sem.my_sem_test

那么你可以发现在 ``/dev/shm`` 目录下存在一个 ``sem.my_sem_test`` 文件，
这就是我们实验中创建的一个信号量，当进程运行完毕，这个信号量将会被删除，
使用sudo权限调用rm命令也可以手动删除该信号量文件。

无名信号量
~~~~~~~~~~

下面的例子是用无名信号量同步机制实现 3 个线程之间的有序执行示例：

.. code-block:: c
    :caption: POSIX无名信号量（base_code/system_programing/posix_sem/sources/posix_sem.c文件）
    :emphasize-lines: 10,13,20,26,31,44,46,61,63,78
    :linenos:

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

本代码说明如下，直接从main函数按流程分析：

-   第44、45行，在for循环内创建了三个信号量存储在数组sem中，创建三个线程，
    线程要调用的函数均为thread_func，并且通过变量i传入了线程序号。
-   第13行，三个线程执行的都是这同样的函数，代码的思路与上一小节有名信号量的示例类似。

    -   第20行，线程先不执行，直接调用sem_wai()等待信号量sem[num]，此处num即创建线程传入的序号参数i。
        即每个线程均等待与自己序号相同的信号量。
    -   第23~27行，得到信号量后在for循环里打印信息并睡眠，释放CPU。
    -   第31行，退出本线程。

-   第58~73行，此时各个子线程已经创建完成，均在等待信号量，这时在原线程里的for循环里调用sem_post()按顺序释放信号量，
    并且调用pthread_join()等待该线程执行完毕再释放下一个信号量。
-   第78行，调用sem_destroy释放各个信号量。

可以推算到如下现象：
在原线程的控制下，它所创建的线程ABC按照释放信号量的次序执行，而且即使上一线程有释放CPU的操作，下一个线程也不会得到CPU的光顾，
因为它未等到自己的信号量。从而在控制下不会出现ACBBAC之类的乱序操作。

实验操作
^^^^^^^^^^^^^^^

本实验的代码存储在 ``base_code/system_programing/posix_sem`` 目录中，编译及运行过程如下：

.. code:: bash

    # 以下操作在 base_code/system_programing/posix_sem 代码目录进行
    # 编译X86版本程序
    make
    # 运行X86版本程序
    ./build_x86/posix_sem_demo 
    
    # 以下是运行的输出
    Create treads success
    Waiting for threads to finish...
    Thread 0 is starting
            Thread 0: job 0 
            Thread 0: job 1 
            Thread 0: job 2 
            Thread 0: job 3 
    Thread 0 finished
    Thread 0 joined
    Thread 1 is starting
            Thread 1: job 0 
            Thread 1: job 1 
            Thread 1: job 2 
            Thread 1: job 3 
    Thread 1 finished
    Thread 1 joined
    Thread 2 is starting
            Thread 2: job 0 
            Thread 2: job 1 
            Thread 2: job 2 
            Thread 2: job 3 
    Thread 2 finished
    Thread 2 joined

可以看到，三个进程是分别连续打印，而且是按信号量释放的次序执行的。
感兴趣的话可以尝试把线程函数的等待信号量操作sem_wait注释掉，观看实验现象。

注意：无名信号量不会在系统中创建文件，所以无法像有名信号量那样通过 ``ls -l /dev/shm`` 命令查看到。

