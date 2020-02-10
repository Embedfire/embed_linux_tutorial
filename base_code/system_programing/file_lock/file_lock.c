#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int lock_set(int fd, int type);

int main(void)
{
    int fd;
    pid_t result;

    fd = open("hello", O_RDWR | O_CREAT, 0644);
    if (fd < 0)
    {
        printf("open file error!\n");
        exit(1);
    }

    result = fork();

    if(result == -1)
    {
        perror("fork");
        exit(1);
    }

    else if(result == 0)    /** 这是一个子进程 */
    {
        printf("this is a  chiad process,pid is %d!\n",getpid());

        lock_set(fd, F_WRLCK);      /** 给文件上锁 */

        printf("chiad sleep!\n\n");
        
        sleep(2);       /** 休眠2S */

        lock_set(fd, F_UNLCK);  /** 解锁 */

        close(fd);
    }

    else    /** 返回值大于0是父进程 */
    {
        printf("this is a  father process,pid is %d!\n",getpid());
        
        lock_set(fd, F_WRLCK);      /** 给文件上锁 */

        printf("father wait a keyboard enter to unlock file!\n\n");

        getchar();

        lock_set(fd, F_UNLCK);  /** 解锁 */     

        close(fd);
    }

    return 0;
}


int lock_set(int fd, int type)
{
    struct flock old_lock, lock;

    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_type = type;
    lock.l_pid = -1;

    /* 判断文件是否可以上锁 */
    fcntl(fd, F_GETLK, &lock);

    if (lock.l_type != F_UNLCK)
    {
        /* 判断文件不能上锁的原因 */
        if (lock.l_type == F_RDLCK) /* 该文件已有读取锁 */
        {
            printf("Read lock already set by %d\n", lock.l_pid);
        }

        else if (lock.l_type == F_WRLCK) /* 该文件已有写入锁 */
        {
            printf("Write lock already set by %d\n", lock.l_pid);
        }
    }

    /* l_type 可能已被 F_GETLK 修改过 */
    lock.l_type = type;

    /* 根据不同的 type 值进行阻塞式上锁或解锁 */
    if ((fcntl(fd, F_SETLKW, &lock)) < 0)
    {
        printf("Lock failed:type = %d\n", lock.l_type);
        return 1;
    }

    switch(lock.l_type)
    {
        case F_RDLCK:
        {
            printf("Read lock set by %d\n", getpid());break;
        }
            
        case F_WRLCK:
        {
            printf("Write lock set by %d\n", getpid());break;
        }
            
        case F_UNLCK:
        {
            printf("Release lock by %d\n", getpid());
            return 1;   break;
        }
        
        default:
            break;
    }/* end of switch */

    return 0;
}








// int main(void)
// {
//     int fd;

//     pid_t result;
//     /*调用 fork()函数*/

//     fd = open("hello", O_RDWR | O_CREAT, 0644);
//     if (fd < 0)
//     {
//         printf("open file error!\n");
//         exit(1);
//     }

//     result = fork();
//     /*通过 result 的值来判断 fork()函数的返回情况，首先进行出错处理*/
//     if(result == -1)
//     {
//         printf("Fork error\n");
//     }
//     else if (result == 0) /*返回值为 0 代表子进程*/
//     {
//         printf("The returned value is %d\nIn child process!!\nMy PID is %d\n",result,getpid());

//         printf("this is child\n");

//         lock_set(fd, F_WRLCK);  /** 给文件上锁 */

//         getchar();

//         lock_set(fd, F_UNLCK);  /** 解锁 */     

//     }

//     else /*返回值大于 0 代表父进程*/
//     {
//         printf("The returned value is %d\nIn father process!!\nMy PID is %d\n",result,getpid());

//         printf("this is father\n");

//         lock_set(fd, F_WRLCK);  /** 给文件上锁 */

//         sleep(1);

//         lock_set(fd, F_UNLCK);  /** 解锁 */  
//     }
//     return result;
// }