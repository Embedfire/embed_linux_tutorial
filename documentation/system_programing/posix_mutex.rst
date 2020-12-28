POSIX 互斥锁
============

互斥锁的基本概念
----------------

在前面的信号量章节，我们也了解过互斥这个概念，就是当不同进程/线程去访问某个临界资源的时候，就需要进行互斥保护，
这种互斥保护可以看做是一种锁机制，就好比当你去上厕所的时候，你会锁住门，不让别人进来。
在Linux系统中的锁机制是一个比较广泛的概念，而且锁的种类很多，包括互斥锁，文件锁，读写锁等等。

其实信号量也是一种锁，可以使用锁的目的是达到互斥的作用，使得共享资源在同一时间内，
只有能有一个进程或者线程对它进行操作。

整个系统拥有很多临界资源，比如某些全局性的文件，硬件接口等，
但整个Linux系统可以完美运行，这无不依赖锁机制，可以说锁机制是Linux整个系统的精髓所在，
Linux内核都是围绕着同步在运转，它决定了进程/线程什么时候可以访问这个临界资源，在多进程和多线程编程中，
锁起着极其重要的作用。信号量也可以作为锁使用，但是我们今天要讲解的是POSIX标准中的机制——互斥锁（mutex）。
一些资料中会将互斥锁称为互斥量，它们是一样的。

互斥锁和信号量不同的是，它具有互斥锁所有权、递归访问等特性，常用于实现对临界资源的独占式处理，
在任意时刻互斥锁的状态只有两种，开锁或闭锁。

当互斥锁被线程持有时，该互斥锁处于闭锁状态，线程获得互斥锁的所有权。当该线程释放互斥锁时，
该互斥锁处于开锁状态，线程失去该互斥锁的所有权。

也就是说，同时只有一个线程能获取互斥锁，特别地，持有该互斥锁的线程能够再次获得这个锁而不被阻塞，
这就是互斥锁的递归访问，这个特性与一般的信号量有很大的不同，
在信号量中，由于会不存在可用的信号量，线程递归获取信号量时会发生阻塞，最终形成死锁。

死锁就是自己把自己阻塞了，就相当于自己把自己锁在门外，钥匙在屋里。还有一种死锁的的情况是，
两个线程相互阻塞，就好比你家的钥匙在你朋友家，你朋友家的钥匙在你家，然后你们都进不去。

想要避免死锁，最好遵循以下的规则：

-   对共享资源操作前一定要获得锁。
-   完成操作以后一定要释放锁。
-   尽量短时间地占用锁。
-   如果有多个锁, 如获得顺序是ABC连环扣, 释放顺序也应该是ABC。

如果想要实现同步功能（线程与线程间同步），信号量或许是更好的选择，虽然互斥锁也可以用于线程与线程间的同步，
但互斥锁更多的是用于保护资源的互斥。互斥锁可以充当保护资源的令牌，当一个线程希望访问某个资源时，
它必须先获取令牌；当线程使用资源后，必须归还令牌，以便其他线程可以访问该资源。

在信号量中也可以用于保护临界资源，当线程获取信号量后才能开始使用该资源，当不需要使用时就释放信号量，
如此一来其他线程也能获取到信号量从而可用使用资源。但信号量会导致的另一个潜在问题：
当其他线程释放这个信号量的时候，这就不能保证信号量能实现互斥操作了。

互斥锁的使用比较单一，并且它是以锁的形式存在，在初始化的时候，互斥锁处于开锁的状态，
而当被线程持有的时候则立刻转为闭锁的状态。互斥锁更适合于以下场景：

1. 保护临界资源。
2. 线程可能会多次获取互斥锁的情况下。这样可以避免同一线程多次递归持有而造成死锁的问题。

多线程环境下往往存在多个线程竞争同一临界资源的应用场景，互斥锁可被用于对临界资源的保护从而实现独占式访问。
另外，互斥锁可以降低信号量存在的优先级翻转问题带来的影响。

例如，有两个线程需要用串口进行发送数据，假设其硬件资源只有一个，那么两个线程肯定不能同时发送数据，
不然将导致数据错误，那么，就可以用互斥锁对串口资源进行保护，当一个线程正在使用串口的时候，
另一个线程则无法使用串口，等到线程使用串口完毕之后，另外一个线程才能获得串口的使用权。

多线程环境下会存在多个线程访问同一临界资源的场景，该资源会被线程独占处理。
其他线程在资源被占用的情况下不允许对该临界资源进行访问，这个时候就需要用到的互斥锁来进行资源保护，
那么互斥锁是怎样来避免这种冲突？

使用互斥锁处理不同线程对临界资源的同步访问时，线程想要获得互斥锁才能访问资源，如果一旦有线程成功获得了互斥锁，
则互斥锁立即变为闭锁状态，此时其他线程会因为获取不到互斥锁而不能访问该资源，
那么此时线程有2个选择：扭头就走，不进行等待操作；或者一直阻塞在这里等待，直到互斥锁被持有线程释放后，
线程才能获取互斥锁从而得以访问该临界资源，此时互斥锁再次上锁，
如此一来就可以确保同一时刻只有一个线程正在访问这个临界资源，保证了临界资源操作的安全性。

初始化互斥锁
------------

静态初始化
~~~~~~~~~~~~~

在使用互斥锁前需要初始化一个互斥锁，而在POSIX标准中支持互斥锁静态初始化和动态初始化两种方式，
如果是静态初始化的可以通过以下代码实现（选择其中一句即可）：

.. code:: c

        pthread_mutex_t fastmutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_t recmutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

        pthread_mutex_t errchkmutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

pthread_mutex_t是互斥锁的结构体，其实就是定义一个互斥锁结构，并且将其赋值，代表不同的互斥锁，
这3种锁的区别主要在于其他未占有互斥锁的线程在获取互斥锁时是否需要阻塞等待：

-   PTHREAD_MUTEX_INITIALIZER：表示默认的互斥锁，即快速互斥锁。互斥锁被线程1持有时，此时互斥锁处于闭锁状态，
    当线程2尝试获取互斥锁，那么线程2将会阻塞直至持有互斥锁的线程1解锁为止。
-   PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP：递归互斥锁。互斥锁被线程1持有时，线程2尝试获取互斥锁，
    将无法获取成功，并且阻塞等待，而如果是线程1尝试再次获取互斥锁时，将获取成功，并且持有互斥锁的次数加1。
-   PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP：检错互斥锁。这是快速互斥锁的非阻塞版本，它会立即返回一个错误代码（线程不会阻塞）。


动态初始化
~~~~~~~~~~~~~
互斥锁动态初始化可以调用pthread_mutex_init()函数，该函数原型如下：

.. code:: c

        int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

pthread_mutex_init()函数是以动态方式初始化互斥锁的，参数说明如下：

-   mutex则是初始化互斥锁结构的指针，
-   mutexattr是属性参数，它允许我们设置互斥锁的属性，从而属性控制着互斥锁的行为，如果参数mutexattr为NULL，
    则使用默认的互斥锁属性，默认属性为快速互斥锁。

获取互斥锁与释放互斥锁
----------------------

从前文我们也知道，想要访问一个临界资源需要获取互斥锁，获取互斥锁就相当于获得访问资源权限，
就好比你有钥匙才能开你家的门。

当互斥锁处于开锁状态时，线程才能够获取互斥锁，当线程持有了某个互斥锁的时候，
其他线程就无法获取这个互斥锁，需要等到持有互斥锁的线程进行释放后，其他线程才能获取成功，
线程通过互斥锁获取函数来获取互斥锁的所有权。

线程对互斥锁的所有权是独占的，任意时刻互斥锁只能被一个线程持有，如果互斥锁处于开锁状态，
那么获取该互斥锁的线程将成功获得该互斥锁，并拥有互斥锁的所有权；
而如果互斥锁处于闭锁状态，则根据互斥锁的类型做对应的处理，默认情况下是快速互斥锁，
获取该互斥锁的线程将无法获得互斥锁，线程将被阻塞，直到互斥锁被释放，当然，如果是同一个线程重复获取互斥锁，也会导致死锁结果。

获取互斥锁有2个函数，mutex参数指定了要操作的互斥锁：

.. code:: c

        int pthread_mutex_lock(pthread_mutex_t *mutex);

        int pthread_mutex_trylock(pthread_mutex_t *mutex);

        int pthread_mutex_unlock(pthread_mutex_t *mutex);

-   pthread_mutex_lock()函数获得访问临界资源的权限，如果已经有其他线程锁住互斥锁，那么该函数会是线程阻塞指定该互斥锁解锁为止。
-   pthread_mutex_trylock()是pthread_mutex_lock()函数的非阻塞版本，使用它不会阻塞当前线程，如果互斥锁已被占用，
    它会理解返回一个EBUSY错误。
-   访问完共享资源后，一定要通过pthread_mutex_unlock()函数释放占用的互斥锁，以便系统其他线程有机会获取互斥锁，访问该资源。

简单说就是，互斥锁的使用流程应该是：

1.  线程获取互斥锁。
#.  然后访问共享资源。
#.  最后释放互斥锁。

销毁互斥锁
----------


pthread_mutex_destroy()函数用于销毁一个互斥锁，当互斥锁不再使用时，可以用它来销毁，mutex参数指定了要销毁的互斥锁：

.. code:: c

        int pthread_mutex_destroy(pthread_mutex_t *mutex);


互斥锁实验
----------

这个实验主要是验证互斥锁的互斥情况，系统创建3个线程，假设这3个线程中有临界资源被访问，
那么我们希望这3个线程按顺序且不能同时去访问这个临界资源（假设临界资源是调用sleep()函数），
所以我们可以使用互斥锁去限制能访问的线程，获取到互斥锁的线程可以访问临界资源。

代码如下：

.. code-block:: c
    :caption: POSIX无名信号量（base_code/system_programing/mutex/sources/mutex.c文件）
    :emphasize-lines: 9,11,18,32,37,50,54,65,69
    :linenos:

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

本代码说明如下，直接从main函数按流程分析：

-   第45行，调用pthread_mutex_init()动态初始化了一个互斥锁。
-   第54行，在for循环内创建三个线程，线程要调用的函数均为thread_func，并且通过变量num传入了线程序号。
-   第11行，三个线程执行的都是这同样的函数。

    -   第18行，调用pthread_mutex_lock()等待互斥锁，若互斥锁被其它线程占用，线程将阻塞在此处等待。
    -   第29、34行，这两行输出信息的代码均处在互斥锁保护的范围内，所以它们是成对出现，而且不会被需求同样互斥锁的其它线程打断。
    -   第32行，获取到互斥锁后对资源进行操作，此处直接使用sleep模拟，并且释放CPU占用。
    -   第37行，调用pthread_mutex_unlock()解锁，以便其它线程使用临界资源。

-   第65行，此时各个子线程已经创建完成，并且执行，这时在原线程里的for循环里调用pthread_join()等待其它线程执行完毕。
-   第69行，各个线程已执行完毕调用pthread_mutex_destroy()销毁互斥量。

可以推算到如下现象：
在原线程的控制下，它创建的线程ABC，在互斥锁的保护下输出代码里成对的29、34行中的信息，
而且即使保护范围内有释放CPU的sleep操作，另外两个线程也不会得到CPU的光顾，因为它们未得到锁。
从而在控制下出现的输出信息是：即A1A2，B1B2，C1C2，而不会出现 A1B1C1，A2B2C2 之类的打乱信息。

实验操作
^^^^^^^^^^^^^^^

本实验的代码存储在 ``base_code/system_programing/mutex`` 目录中，编译及运行过程如下：

.. code:: bash

    # 以下操作在 base_code/system_programing/mutex 代码目录进行
    # 编译X86版本程序
    make
    # 运行X86版本程序
    ./build_x86/mutex_demo
    
    # 以下是运行的输出
    Thread 0 is hold mutex
    Thread 0 freed mutex

    Thread 1 is hold mutex
    Thread 1 freed mutex

    Thread 2 is hold mutex
    Thread 2 freed mutex

如果觉得不太明确的话，可以将互斥锁的相关代码注释掉，即注释掉18~27行代码，
看看如果没有互斥操作的话线程会怎样访问临界资源，再重新编译运行，运行结果如下：

.. code:: bash

    # 注释掉18~27行代码后重新编译运行
    # 以下操作在 base_code/system_programing/mutex 代码目录进行
    # 编译X86版本程序
    make
    # 运行X86版本程序
    ./build_x86/mutex_demo
    
    # 注释掉18~27行代码后重新编译运行后的输出

    Thread 0 is hold mutex
    Thread 1 is hold mutex
    Thread 2 is hold mutex
    Thread 0 freed mutex

    Thread 2 freed mutex

    Thread 1 freed mutex

可以看到，由于没有锁的保护，在sleep期间其它线程得到了CPU，也对资源进行了操作，
此处出现了A1C1B1，A2C2B2的信息打印顺序。就表示了线程A还在使用关键资源的时候，
线程B和C也同时使用了，如果资源只有一个，那就会出现不可预知的问题。
