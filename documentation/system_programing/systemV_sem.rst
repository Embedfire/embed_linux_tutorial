system-V IPC 信号量
===================

**提示：本章主要讲解进程信号量的内容，如非特别说明，“信号量”均表示system-VIPC信号量** ，
这主要是为了区分后续章节的“POSIX信号量”。

进程信号量基本概念
------------------

信号量与已经介绍过的信号、管道、FIFO以及消息列队不同，
它本质上是一个计数器，用于协调多进程间对共享数据对象的读取，它不以传送数据为主要目的，
它主要是用来保护共享资源（信号量也属于临界资源），使得该临界资源在一个时刻只有一个进程独享。
可能会有同学问了，为什么不使用全局变量呢？那是因为全局变量并不能在进程间共同使用，
因为进程间是相互独立的，而且也无法保证引用计数的原子操作，因此使用系统提供的信号量即可。

信号量的工作原理
----------------

由于信号量只能进行两种操作：等待和发送信号，即P操作和V操作，锁行为就是P操作，解锁就是V操作，
可以直接理解为P操作是申请资源，V操作是释放资源。
PV操作是计算机操作系统需要提供的基本功能之一，它们的行为是这样的：

-   P 操作：如果有可用的资源（信号量值大于0），则占用一个资源（给信号量值减去一，进入临界区代码）;
    如果没有可用的资源（信号量值等于 0），则阻塞，直到系统将资源分配给该进程（进入等待队列，一直等到资源轮到该进程）。
    这就像你要把车开进停车场之前，先要向保安申请一张停车卡一样，P 操作就是申请资源，如果申请成功，
    资源数（空闲的停车位）将会减少一个，如果申请失败，要不在门口等，要不就走人。

-   V   操作：如果在该信号量的等待队列中有进程在等待资源，则唤醒一个阻塞的进程。如果没有进程等待它，
    则释放一个资源（给信号量值加一），就跟你从停车场出去的时候一样，空闲的停车位就会增加一个。

举个例子，就是两个进程共享信号量sem，sem可用信号量的数值为1（资源数为1），一旦其中一个进程执行了P操作，它将得到信号量，
并可以进入临界区，使sem减1。而第二个进程将被阻止进入临界区，因为当它试图执行P操作时，sem为0，
它会被挂起以等待第一个进程离开临界区域并执行V操作释放了信号量，这时第二个进程就可以恢复执行。

在信号量进行PV操作时都为原子操作（因为它需要保护临界资源）。

    注：原子操作：单指令的操作称为原子的，单条指令的执行是不会被打断的

简单来说就是内核可以对这个信号量（计数器）做加减操作，并且操作时遵守一些基本操作原则，
即：对计数器做加操作立即返回，做减操作要检查计数器当前值是否可减？（这个计数器的值要大于1），
如果是则进行减操作；否则将进程将阻塞等待，直到系统中有进程对该信号量进行P操作。

创建或获取一个信号量
--------------------

semget()获取函数
~~~~~~~~~~~~~~~

semget()函数的功能是创建或者获取一个已经创建的信号量，如果成功则返回对应的信号量标识符，
失败则返回-1。函数原型如下：

.. code:: c

        int semget(key_t key, int nsems, int semflg);

参数说明：

-   key：与消息队列一样的是，参数key用来标识系统内的信号量，
    如果指定的key已经存在，则意味着打开这个信号量，这时nsems参数指定为0，semflg参数也指定为0。
    特别地，可以使用IPC_PRIVATE创建一个没有key的信号量。
-   nsems：本参数用于在创建信号量的时候，表示可用的信号量数目。
-   semflg：semflg参数用来指定标志位，与消息队列中的类似。
    主要有IPC_CREAT，IPC_EXCL和权限mode，其中使用IPC_CREAT标志创建新的信号量，
    即使该信号量已经存在（具有同一个键值的信号量已在系统中存在），也不会出错。
    如果同时使用IPC_EXCL标志可以创建一个新的唯一的信号量，此时如果该信号量已经存在，
    该函数会返回出错。

创建信号量时，还受到以下系统信息的影响:

-   SEMMNI：系统中信号量总数的最大值。 
-   SEMMSL：每个信号量中信号量元素个数的最大值。 
-   SEMMNS：系统中所有信号量中的信号量元素总数的最大值。

在Linux系统中，以上信息可通过命令 ``ipcs -l`` 查看.

信号量操作
----------

semop() PV操作函数
~~~~~~~~~~~~~~~~~~~~~
Linux提供了semop()函数对信号量进行PV操作。函数原型如下：

.. code:: c

        int semop(int semid, struct sembuf *sops, size_t nsops);


参数说明：

-   semid：System V信号量的标识符，用来标识一个信号量。 
-   sops：是指向一个struct sembuf结构体数组的指针，该数组是一个信号量操作数组。原型如下：

    .. code:: c

        struct sembuf
        {
            unsigned short int sem_num;   /* 信号量的序号从0 ~ nsems-1 */
            short int sem_op;            /* 对信号量的操作，>0, 0, <0 */
            short int sem_flg;            /* 操作标识：0， IPC_WAIT, SEM_UNDO */
        };

    -   sem_num用于标识信号量中的第几个信号量，0表示第1个，1表示第2个，nsems -1表示最后一个。

    -   sem_op标识对信号量的所进行的操作类型。对信号量的操作有三种类型：

        -   sem_op 大于 0，表示进程对资源使用完毕，交回该资源，即对该信号量执行V操作，交回的资源数由sem_op决定，
            系统会把sem_op的值加到该信号量的信号量当前值semval上。
            特别地，如果sem_flag指定了SEM_UNDO（还原）标志，则从该进程的此信号量调整值中减去sem_op。

        -  sem_op 小于 0，表示进程希望使用资源，对该信号量执行P操作，
            当信号量当前值semval 大于或者等于 -sem_op时，semval减掉sem_op的绝对值，
            为该进程分配对应数目的资源。特别地，如果指定SEM_UNDO，则sem_op的绝对值也加到该进程的此信号量调整值上。
            当semval 小于 -sem_op时，相应信号量的等待进程数量就加1，调用进程被阻塞，
            直到semval 大于或者等于 -sem_op 时，调用进程被唤醒，执行相应的P操作。

        -   sem_op 等于 0，表示进程要阻塞等待，直至信号量当前值semval 变为 0。

    -   sem_flg，信号量操作的属性标志，可以指定的参数包括IPC_NOWAIT和SEM_UNDO。如果为0，
        表示正常操作；当指定了SEM_UNDO，那么将维护进程对信号量的调整值，进程退出的时候会自动还原它对信号量的操作；
        当指定了IPC_WAIT，使对信号量的操作时非阻塞的。即指定了该标志，调用进程在信号量的值不满足条件的情况下不会被阻塞，
        而是直接返回-1，并将errno设置为EAGAIN。

        那么什么是信号量调整值呢？其实就是指定信号量针对某个特定进程的调整值。只有sembuf结构的sem_flag指定为SEM_UNDO后，
        信号量调整值才会随着sem_op而更新。讲简单一点：对某个进程，在指定SEM_UNDO后，对信号量的当前值的修改都会反应到信号量调整值上，
        当该进程终止的时候，内核会根据信号量调整值重新恢复信号量之前的值，SEM_UNDO操作可以防止进程退出时没有释放信号量导致的死锁。


-   nsops：表示上面sops数组的数量，如只有一个sops数组，nsops就设置为1。
    

semctl()属性函数
~~~~~~~~~~~~~~~~~~~~~

semctl函数主要是对信号量集的一系列控制操作，根据操作命令cmd的不同，执行不同的操作，第四个参数是可选的。原型如下：

.. code:: c

        int semctl(int semid, int semnum, int cmd, ...);


-   semid：System V信号量的标识符；

-   semnum：表示信号量集中的第semnum个信号量。它的取值范围： ``0 ~ nsems-1`` 。

-   cmd：操作命令，主要有以下命令：

    -   IPC_STAT：获取此信号量集合的semid_ds结构，存放在第四个参数的buf中。
    -   IPC_SET：通过第四个参数的buf来设定信号量集相关联的semid_ds中信号量集合权限为sem_perm中的uid，gid，mode。
    -   IPC_RMID：从系统中删除该信号量集合。
    -   GETVAL：返回第semnum个信号量的值。
    -   SETVAL：设置第semnum个信号量的值，该值由第四个参数中的val指定。
    -   GETPID：返回第semnum个信号量的sempid，最后一个操作的pid。
    -   GETNCNT：返回第semnum个信号量的semncnt。等待semval变为大于当前值的线程数。
    -   GETZCNT：返回第semnum个信号量的semzcnt。等待semval变为0的线程数。
    -   GETALL：去信号量集合中所有信号量的值，将结果存放到的array所指向的数组。
    -   SETALL：按arg.array所指向的数组中的值，设置集合中所有信号量的值。

-  第四个参数是可选的：如果使用该参数，该参数的类型为 union semun，它是多个特定命令的联合体，具体如下：

.. code:: c

        union semun {
            int              val;    /* Value for SETVAL */
            struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
            unsigned short  *array;  /* Array for GETALL, SETALL */
            struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                        (Linux-specific) */
        };

信号量示例
----------

因为system V的信号量相关的函数调用接口比较复杂，本示例将其封装成单个信号量的几个基本函数。
这些函数的实现单独作为sem.c文件的内容，同时还实现一个sem.h作为外部调用的头文件。具体实现如下所示:

.. code-block:: c
    :caption: 封装信号量操作（base_code/system_programing/systemV_sem/sources/sem.c文件）
    :emphasize-lines: 18,21,34,46,49,62,65
    :linenos:

    #include <sys/sem.h>
    #include <sys/ipc.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <sys/shm.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <errno.h>

    #include "sem.h"


    /* 信号量初始化（赋值）函数*/
    int init_sem(int sem_id, int init_value)
    {
        union semun sem_union;
        sem_union.val = init_value; /* init_value 为初始值 */

        if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
        {
            perror("Initialize semaphore");
            return -1;
        }

        return 0;
    }

    /* 从系统中删除信号量的函数 */
    int del_sem(int sem_id)
    {
        union semun sem_union;
        if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        {
            perror("Delete semaphore");
            return -1;
        }
    }

    /* P 操作函数 */
    int sem_p(int sem_id)
    {
        struct sembuf sops;
        sops.sem_num = 0; /* 单个信号量的编号应该为 0 */
        sops.sem_op = -1; /* 表示 P 操作 */
        sops.sem_flg = SEM_UNDO; /* 若进程退出，系统将还原信号量*/

        if (semop(sem_id, &sops, 1) == -1)
        {
            perror("P operation");
            return -1;
        }
        return 0;
    }

    /* V 操作函数*/
    int sem_v(int sem_id)
    {
        struct sembuf sops;
        sops.sem_num = 0; /* 单个信号量的编号应该为 0 */
        sops.sem_op = 1; /* 表示 V 操作 */
        sops.sem_flg = SEM_UNDO; /* 若进程退出，系统将还原信号量*/

        if (semop(sem_id, &sops, 1) == -1)
        {
            perror("V operation");
            return -1;
        }
        return 0;
    }

它们分别为信号量初始化函数sem_init()、删除信号量的函数sem_del()、P 操作函数 sem_p()以及 V 操作函数 sem_v()。
具体说明如下：

-   sem_init：初始化函数，根据给定的参数设置信号量的初始值，用于设置初始可用资源数。
    函数的内部通过调用semctl()使用SETVAL命令设置semun类型的sem_union变量，该变量中包含了信号量初始值。
-   sem_del：删除信号量函数，通过调用semctl()使用IPC_RMID命令删除指定的信号量。
-   sem_p：P 操作函数，调用semop()设置调整值，其中的sops.sem_op值为-1，表示每次P操作使信号量的值减1。
-   sem_p：V 操作函数，调用semop()设置调整值，它与P操作函数的差异是sops.sem_op的值为+1，表示每次V操作使信号量的值加1。




利用上面封装的操作函数，编写以下测试示例，首先创建一个子进程，
接下来使用信号量来控制两个进程（父子进程）之间的执行顺序。

.. code-block:: c
    :caption: 封装信号量操作头文件（base_code/system_programing/systemV_sem/sources/test.h文件）
    :emphasize-lines: 22,32,41
    :linenos:

    #include <sys/types.h>
    #include <sys/shm.h>
    #include <sys/sem.h>
    #include <sys/ipc.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <errno.h>

    #include "sem.h"

    #define DELAY_TIME 3 /* 为了突出演示效果，等待几秒钟， */

    int main(void)
    {
        pid_t result;
        int sem_id;

        sem_id = semget((key_t)6666, 1, 0666 | IPC_CREAT); /* 创建一个信号量*/

        init_sem(sem_id, 0);

        /*调用 fork()函数*/
        result = fork();
        if(result == -1)
        {
            perror("Fork\n");
        }
        else if (result == 0) /*返回值为 0 代表子进程*/
        {
            printf("Child process will wait for some seconds...\n");
            sleep(DELAY_TIME);
            printf("The returned value is %d in the child process(PID = %d)\n",result, getpid());

            sem_v(sem_id);
        }

        else /*返回值大于 0 代表父进程*/
        {
            sem_p(sem_id);
            printf("The returned value is %d in the father process(PID = %d)\n",result, getpid());

            sem_v(sem_id);

            del_sem(sem_id);
        }

        exit(0);
    }

代码说明如下：

-   第22行：调用semget()创建一个信号量，权限为0666，任何用户均可读写。
-   第24行：调用init_sem()初始化信号量值为0。
-   第27行，使用fork创建子进程。
-   第32~38行，子进程先睡眠一定时间，结束睡眠后通过sem_v给信号量加1。
-   第41~49行，父进程通过sem_p()等待信号量，得到信号量后才输出信息。

本例子的结果是，使父进程在子进程释放信号量后才运行，模拟了一个进程创建资源，一个进程等待资源的协调过程。

实验操作
~~~~~~~~~~~~~~~~~~~~~

本实验的代码存储在base_code/system_programing/systemV_sem目录中，编译及运行过程如下：

.. code:: bash

    # 以下操作在 system_programing/systemV_sem代码目录进行
    # 编译X86版本程序
    make
    # 运行X86版本程序
    ./build_x86/systemV_sem_demo 
    
    # 以下是运行的输出
    Child process will wait for some seconds...
    # 子进程等待一段时间才释放信号量
    The returned value is 0 in the child process(PID = 16085)
    # 父进程等待子进程释放信号量后才打印
    The returned value is 16085 in the father process(PID = 16084)

运行的现象就是

