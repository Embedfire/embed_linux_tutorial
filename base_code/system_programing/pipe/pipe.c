/** pipe.c */

/**
 * ps –ef | grep ntp
 * 
 * 进程 ps -ef              进程 grep ntp
 *          |              |
 *          |---- pipe ----|
 * 
 * 管道是 Linux 中进程间通信的一种方式。这里所说的管道主要指无名管道，它具有如下特点:
 *   它只能用于具有亲缘关系的进程之间的通信（也就是父子进程或者兄弟进程之间）。
 *   它是一个半双工的通信模式，具有固定的读端和写端。
 *   管道也可以看成是一种特殊的文件，对于它的读写也可以使用普通的 read()和 write()等函数。
 *    但是它不是普通的文件，并不属于其他任何文件系统，并且只存在于内核的内存空间中。
 * 
 * 管道是基于文件描述符的通信方式，当一个管道建立时，它会创建两个文件描述符 fds[0]和 fds[1]，
 * 其中 fds[0]固定用于读管道，而 fd[1]固定用于写管道，这样就构成了一个半双工的通道。
 * 管道关闭时只需将这两个文件描述符关闭即可，可使用普通的 close()函数逐个关闭各个文件描述符。
 * 
 *           用户进程
 *  fd[0]               fd[1]
 *    |                   |
 *    读                  写
 *    |--------管道--------|
 * 
 *     父进程                 子进程
 *  fd[0]fd[1]            fd[0]fd[1] 
 *      |                     |
 *     读写                   读写
 *      |---------管道---------|
 * 
 * 函数原型：int pipe(int fd[2])
 * 
 */

/**
 * 注意点：
 *   只有在管道的读端存在时，向管道写入数据才有意义。否则，向管道写入数据的进程将收到内核传来的 SIGPIPE 信号（通常为 Broken pipe 错误）。
 *   向管道写入数据时， Linux 将不保证写入的原子性，管道缓冲区一有空闲区域，写进程就会试图向管道写入数据。如果读进程不读取管道缓冲区中的数据，那么写操作将会一直阻塞。
 *   父子进程在运行时，它们的先后次序并不能保证，建议使用进程之间的同步与互斥机制之后。
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DATA_LEN 256
#define DELAY_TIME 1

int main()
{
    pid_t pid;
    int pipe_fd[2];
    char buf[MAX_DATA_LEN];
    const char data[] = "Pipe Test Program";
    int real_read, real_write;

    memset((void*)buf, 0, sizeof(buf));

    /* 创建管道 */
    if (pipe(pipe_fd) < 0)
    {
        printf("pipe create error\n");
        exit(1);
    }

    /* 创建一子进程 */
    if ((pid = fork()) == 0)
    {
        /* 子进程关闭写描述符，并通过使子进程暂停 3s 等待父进程已关闭相应的读描述符 */
        close(pipe_fd[1]);
        sleep(DELAY_TIME * 3);

        /* 子进程读取管道内容 */
        if ((real_read = read(pipe_fd[0], buf, MAX_DATA_LEN)) > 0)
        {
            printf("%d bytes read from the pipe is '%s'\n", real_read, buf);
        }

        /* 关闭子进程读描述符 */
        close(pipe_fd[0]);

        exit(0);
    }
    
    else if (pid > 0)
    {
        /* 父进程关闭读描述符，并通过使父进程暂停 1s 等待子进程已关闭相应的写描述符 */
        close(pipe_fd[0]);

        sleep(DELAY_TIME);
        
        if((real_write = write(pipe_fd[1], data, strlen(data))) != -1)
        {
            printf("Parent write %d bytes : '%s'\n", real_write, data);
        }
        
        /*关闭父进程写描述符*/
        close(pipe_fd[1]);

        /*收集子进程退出信息*/
        waitpid(pid, NULL, 0);

        exit(0);
    }
}
