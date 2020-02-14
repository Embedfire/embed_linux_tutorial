/** sem.c */

/**
 * 多进程编程中需要关注进程间的同步及互斥问题。同步是指多个进程为了完成同一个任务相互协作运行，而互斥是指不同的进程为了争夺有限的系统资源（硬件或软件资源）而相互竞争运行。
 * 信号量是用来解决进程间同步与互斥问题的一种进程间通信机制，它是一个特殊的变量，变量的值代表着关联资源的可用数量。 若等于 0 则意味着目前没有可用的资源。
 * 根据信号量的值可以将信号量分为二值信号量和计数信号量：
 *   二值信号量： 信号量只有 0 和 1 两种值。 若资源被锁住，信号量值为 0，若资源可用则信号量值为 1；
 *   计数信号量： 信号量可在 0 到一个大于 1 的数（最大 32767） 之间取值。该计数表示可用资源的个数。
 * 信号量只能进行两个原子操作： P 操作： V 操作。
 *   P 操作：如果有可用的资源（信号量值>0），则占用一个资源（将信号量值减 1）；如果没有可用的资源（信号量值=0），则进程被阻塞直到系统将资源分配给该进程（进入信号量的等待队列，等到资源后再唤醒该进程）
 *   V 操作：如果在该信号量的等待队列中有进程在等待资源，则唤醒一个阻塞进程；如果没有进程等待它，则释放一个资源（给信号量值加 1）
 * 
 * POSIX 提供两类信号量： 有名信号量和基于内存的信号量（也称无名信号量）。有名信号量可以让不同的进程通过信号量的名字获取到信号量，
 * 而基于内存的信号量只能放置在进程间共享内存区域中。有名信号量与基于内存的信号量的初始化和销毁方式不同。
 * 
 * 创建或打开有名信号量：
 * sem_t *sem_open(const char *name, int oflag);
 * sem_t *sem_open(const char *name, int oflag,mode_t mode, unsigned int value);
 * 信号量的类型为 sem_t， 该结构里记录着当前共享资源的数目。 sem_open()函数成功返回指向信号量的指针，失败返回 SEM_FAILED
 * 函数参数如下：
 *  参数 name 为信号量的名字，两个不同的进程可以通过传递相同的名字打开同一个信号量；
 *  oflag 可以是以下标志的或值：
 *   O_CREAT：如果name指定的信号量不存在则创建，此时必须给出 mode和value值；
 *   O_EXCL：如果 name 指定的信号量存在，而 oflag 指定为 O_CREAT | O_EXCL，则 sem_open()函数返回错误。
 *  mode 为信号量的权限位，类似于 open()函数；
 *  value 为信号量的初始化值。
 * 
 * 关闭有名信号量,成功返回 0，失败返回-1:
 * int sem_close(sem_t *sem); 
 * 
 * 删除有名信号量：
 * int sem_unlink(const char *name); 
 * 
 * 
 * 初始化基于内存信号量：
 * int sem_init(sem_t *sem, int pshared, unsigned int value);
 * 函数成功返回 0，失败返回-1；
 * 参数 sem 为需要初始化的信号量的指针； pshared 值如果为 0 表示该信号量只能在线程内部使用，否则为进程间使用。
 * 在进程间使用时，该信号量需要放在共享内存处； value 为信号量的初始化值，代表的资源数。
 * 
 * 销毁基于内存信号量：
 * int sem_destroy(sem_t *sem);
 * 该函数只能销毁由 sem_init()初始化的信号量，函数成功返回 0，否则返回-1。参数 sem 指出需要销毁的信号量。
 */


/**
 * 信号量的 P 操作由 sem_wait()函数来完成:
 * int sem_wait(sem_t *sem);
 * 如果信号量的值大于 0， sem_wait()函数将信号量值减 1 并立即返回，代表着获取到资源，如果信号量值等于 0 则调用进程（线程）将进入睡眠状态，直到该值变为大于 0 时再将它减 1 后才返回。
 * 函数成功返回 0，否则返回-1；参数 sem 为需要操作的信号量。
 * 
 * 信号量的 V 操作有 sem_post()函数来完成:
 * int sem_post(sem_t *sem);
 * 当一个进程（线程） 使用完某个信号量时，它应该调用 sem_post()来告诉系统申请的资源已经使用完毕。 sem_post()函数与 sem_wait()函数的功能正好相反，
 * 它把所指定的信号量的值加 1，然后唤醒正在等待该信号量的任意进程（线程） 。
 * 函数成功返回 0，否则返回-1；参数 sem 为需要操作的信号量。
 */
/** 通过信号量同步源码见共享内存章节(以上信号量接口是POSIX接口) */


/**
 * 头文件：
 * #include <sys/ipc.h>
 * #include <sys/sem.h>
 * 
 * 在 Linux 系统中，使用信号量通常分为以下几个步骤：
 * （1）创建信号量或获得在系统已存在的信号量，此时需要调用 semget()函数。不同进程通过使用同一个信号量键值来获得同一个信号量。
 * （2）初始化信号量，此时使用 semctl()函数的 SETVAL 操作。当使用二值信号量时，通常将信号量初始化为 1。
 * （3）进行信号量的 PV 操作，此时调用 semop()函数。这一步是实现进程之间的同步和互斥的核心工作部分。
 * （4）如果不需要信号量，则从系统中删除它，此时使用 semctl()函数的 IPC_RMID 操作。此时需要注意，在程序中不应该出现对已经被删除的信号量的操作。
 * 
 * 函数原型 int semget(key_t key, int nsems, int semflg)
 * 参数:
 *  key：信号量的键值，多个进程可以通过它访问同一个信号量，其中有个特殊值 IPC_PRIVATE。它用于创建当前进程的私有信号量
 *  nsems：需要创建的信号量数目，通常取值为 1
 *  semflg：同 open()函数的权限位，也可以用八进制表示法，其中使用IPC_CREAT 标志创建新的信号量，即使该信号量已经存在具有同一个键值的信号量
 *          已在系统中存在），也不会出错。如果同时使用 IPC_EXCL 标志可以创建一个新的唯一的信号量，此时如果该信号量已经存在，该函数会返回出错
 * 返回
 *  成功：信号量标识符，在信号量的其他函数中都会使用该值
 *  出错：-1
 * 
 * 函数原型 int semctl(int semid, int semnum, int cmd, union semun arg)
 * 函数传入值：
 * semid： semget()函数返回的信号量标识符
 * semnum：信号量编号，当使用信号量集时才会被用到。通常取值为 0，就是使用单个信号量（也是第一个信号量）
 * cmd：指定对信号量的各种操作，当使用单个信号量（而不是信号量集）时，常用的有以下几种：
 *  IPC_STAT：获得该信号量（或者信号量集合）的 semid_ds 结构，并存放在由第 4 个参数 arg 的 buf 指向的 semid_ds 结构中。 semid_ds 是在系统中描述信号量的数据结构。
 *  IPC_SETVAL：将信号量值设置为 arg 的 val 值
 *  IPC_GETVAL：返回信号量的当前值
 *  IPC_RMID：从系统中，删除信号量（或者信号量集）
 * arg：是 union semnn 结构，该结构可能在某些系统中并不给出定义，此时必须由程序员自己定义
 * union semun
 * {
 *  int val;
 *  struct semid_ds *buf;
 *  unsigned short *array;
 * }
 * 函数返回值
 *  成功：根据 cmd 值的不同而返回不同的值
 *    IPC_STAT、 IPC_SETVAL、 IPC_RMID：返回 0
 *    IPC_GETVAL：返回信号量的当前值
 *  出错： -1
 * 
 * 函数原型 int semop(int semid, struct sembuf *sops, size_t nsops)
 * 函数传入值
 * semid： semget()函数返回的信号量标识符
 * sops：指向信号量操作数组，一个数组包括以下成员：
 * struct sembuf
 * {
 *  short sem_num;  信号量编号，使用单个信号量时，通常取值为 0
 *  short sem_op;   信号量操作：取值为-1 则表示 P 操作，取值为+1 则表示 V 操作
 *  short sem_flg;  通常设置为 SEM_UNDO。这样在进程没释放信号量而退出时，系统自动释放该进程中未释放的信号量
 * }
 * nsops：操作数组 sops 中的操作个数（元素数目），通常取值为 1（一个操作）
 * 函数返回值 成功：信号量标识符，在信号量的其他函数中都会使用该值
 */


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
    struct sembuf sem_b;
    sem_b.sem_num = 0; /* 单个信号量的编号应该为 0 */
    sem_b.sem_op = -1; /* 表示 P 操作 */
    sem_b.sem_flg = SEM_UNDO; /* 系统自动释放将会在系统中残留的信号量*/

    if (semop(sem_id, &sem_b, 1) == -1)
    {
        perror("P operation");
        return -1;
    }
    return 0;
}

/* V 操作函数*/
int sem_v(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0; /* 单个信号量的编号应该为 0 */
    sem_b.sem_op = 1; /* 表示 V 操作 */
    sem_b.sem_flg = SEM_UNDO; /* 系统自动释放将会在系统中残留的信号量*/

    if (semop(sem_id, &sem_b, 1) == -1)
    {
        perror("V operation");
        return -1;
    }
    return 0;
}
