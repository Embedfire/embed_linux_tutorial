/** shm.c(share memory) */

/**
 * 共享内存是允许两个不相关的进程访问同一个逻辑内存的进程间通信方法，是在两个正在运行的进程之间共享和传递数据的一种非常有效的方式。
 * 
 * 共享内存的实现分为两个步骤，第一步是创建共享内存，这里用到的函数是 shmget()，也就是从内存中获得一段共享内存区域;
 * 第二步映射共享内存，也就是把这段创建的共享内存映射到具体的进程空间中，这里使用的函数是 shmat()。
 * 到这里，就可以使用这段共享内存了，也就是可以使用不带缓冲的 I/O 读写命令对其进行操作。
 * 除此之外，当然还有撤销映射的操作，其函数为 shmdt()。
 * 
 * 函数原型: int shmget(key_t key, int size, int shmflg)
 * 函数传入值:
 *  key：共享内存的键值，多个进程可以通过它访问同一个共享内存，如果指定为 IPC_PRIVATE ，则会自动产生一个随机未用的新键值。
 *  size：共享内存区大小
 *  shmflg：
 *     IPC_CREAT ：共享内存不存在才创建；
 *     IPC_EXCL ：如果指定了 IPC_CREAT，但共享内存已经存在时返回错误；
 *     SHM_HUGETLB ：使用大页面分配共享内存
 *    同 open()函数的权限位，也可以用八进制表示法
 * 函数返回值:
 *  成功：共享内存段标识符
 *  出错： -1
 * 
 * 函数原型 void *shmat(int shmid, const void *shmaddr, int shmflg)
 * 函数传入值:
 *  shmid：要映射的共享内存区标识符(ID)
 *  shmaddr：将共享内存映射到指定地址（若为 NULL 则表示系统自动分配地址并把该段共享内存映射到调用进程的地址空间）
 *  shmflg:
 *    SHM_RDONLY： 共享内存只读
 *    默认 0：共享内存可读写
 * 函数返回值
 *  成功：被映射的段地址
 *  出错： -1
 * 
 * 函数原型 int shmdt(const void *shmaddr)
 * 函数传入值: shmaddr：被映射的共享内存段地址
 * 函数返回值: 
 *  成功： 0
 *  出错： -1
 * 
 * 获取或设置共享内存相关信息
 * 原型:int shmctl(int shmid，int cmd，struct shmid_ds*buf)
 * shmid :共享内存ID
 * cmd:
 *  IPC_STAT :获取属性信息，放置到buf中
 *  IPC_SET :设置属性信息为buf指向的内容
 *  IPC_RMID :将共享内存标记为“即将被删除”状态
 *  IPC_INFO :获得关于共享内存的系统限制值信息
 *  SHM_INFo :获得系统为共享内存消耗的资源信息
 *  SHM_STAT :同IPC_STAT，但shmid为该SHM在内核中记录所有SHM信息的数组的下标，因此通过迭代所有的下标可以获得系统中所有SHM的相关信息
 *  SHM_LOCK :禁止系统将该SHM交换至swap分区
 *  SHM_UNLOCK :允许系统将该SHM交换至swap分区buf属性信息结构体指针
 * 返回值
 *  IPC_INFO :内核中记录所有SHM信息的数组的下标最大值成功
 *  SHM_INFO :内核中记录所有SHM信息的数组的下标最大值成功
 *  SHM_STAT :下标值为shmid的SHM的ID
 *  失败:-1
 * 
 * 共享内存在父子进程间遵循的约定
 * 1.使用 fork() 函数创建一个子进程后，该进程继承父亲进程挂载的共享内存。
 * 2.如果调用 exec() 执行一个新的程序，则所有挂载的共享内存将被自动卸载。
 * 3.如果在某个进程中调用了 exit() 函数，所有挂载的共享内存将与当前进程脱离关系。
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SHM_SIZE 2048

int main()
{
    int shm_id, pid; 
    int *ptr = NULL;

    struct shmid_ds buf;

    buf.

    /** 创建一个共享内存 */
    shm_id = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT|0644);

    /** 映射共享内存 */
    ptr = (int *)shmat(shm_id, NULL, 0);

    printf("Attach addr is %p \n", ptr);

    *ptr = 1004;

    printf("The Value of Parent is : %d \n", *ptr);

    if((pid=fork()) == -1)
    {
        perror("fork Err");
        exit(0);
    }

    else if(!pid)
    {
        printf("The Value of Child is : %d \n", *ptr);
        exit(0);
    }

    else
    {
        sleep(1);

        /* 解除映射 */
        shmdt(ptr);
        
        /* 删除共享内存 */
        shmctl(shm_id, IPC_RMID, 0);
    }

    return 0;
}
