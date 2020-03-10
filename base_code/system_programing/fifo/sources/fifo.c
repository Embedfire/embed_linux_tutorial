/**fifo.c */

/**
 * 无名管道，它只能用于具有亲缘关系的进程之间，这就大大地限制了管道的使用。有名管道的出现突破了这种限制， 它可以使互不相关的两个进程实现彼此通信。 
 * 该管道可以通过路径名来指出，并且在文件系统中是可见的。在建立了管道之后，两个进程就可以把它当作普通文件一样进行读写操作，使用非常方便。
 * 不过 FIFO 是严格地遵循先进先出规则的，对管道及 FIFO 的读总是从开始处返回数据，对它们的写则把数据添加到末尾，它们不支持如 lseek()等文件定位操作。
 * 有名管道的创建可以使用函数 mkfifo()，该函数类似文件中的 open()操作，可以指定管道的路径和打开的模式
 * 
 * 在创建管道成功之后，就可以使用 open()、 read()和 write()这些函数了。与普通文件的开发设置一样，对于为读而打开的管道可在 open()中设置 O_RDONLY，
 * 对于为写而打开的管道可在 open()中设置 O_WRONLY，在这里与普通文件不同的是阻塞问题。由于普通文件的读写时不会出现阻塞问题，而在管道的读写中却有阻塞的可能，
 * 这里的非阻塞标志可以在 open()函数中设定为 O_NONBLOCK。下面分别对阻塞打开和非阻塞打开的读写进行讨论。
 * （1）对于读进程。
 *       若该管道是阻塞打开，且当前 FIFO 内没有数据，则对读进程而言将一直阻塞到有数据写入。
 *       若该管道是非阻塞打开，则不论 FIFO 内是否有数据，读进程都会立即执行读操作。即如果 FIFO内没有数据，则读函数将立刻返回 0。
 * （2）对于写进程。
 *       若该管道是阻塞打开，则写操作将一直阻塞到数据可以被写入。
 *       若该管道是非阻塞打开而不能写入全部数据，则读操作进行部分写入或者调用失败
 * 
 * 函数原型 int mkfifo(const char *filename,mode_t mode)
 * 函数传入值 filename：要创建的管道名字(包含路径)
 * 函数传入值 mode：
 *       O_RDONLY：读管道
 *       O_WRONLY：写管道
 *       O_RDWR：读写管道
 *       O_NONBLOCK：非阻塞
 *       O_CREAT：如果该文件不存在，那么就创建一个新的文件，并用第三个参数为其设置权限
 *       O_EXCL：如果使用 O_CREAT 时文件存在，那么可返回错误消息。这一参数可测试文件是否存在
 * 函数返回值：
 *       0：成功
 *       EACCESS：参数 filename 所指定的目录路径无可执行的权限
 *       EEXIST：参数 filename 所指定的文件已存在
 *       ENAMETOOLONG：参数 filename 的路径名称太长
 *       ENOENT：参数 filename 包含的目录不存在
 *       ENOSPC：文件系统的剩余空间不足
 *       ENOTDIR：参数 filename 路径中的目录存在但却非真正的目录
 *       EROFS：参数 filename 指定的文件存在于只读文件系统内
 */
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>


#define MYFIFO "myfifo"    /* 有名管道文件名*/

#define MAX_BUFFER_SIZE PIPE_BUF /* 4096 定义在于 limits.h 中*/


void fifo_read(void)
{
    char buff[MAX_BUFFER_SIZE];
    int fd;
    int nread;

    printf("***************** read fifo ************************\n");
    /* 判断有名管道是否已存在，若尚未创建，则以相应的权限创建*/
    if (access(MYFIFO, F_OK) == -1)
    {
        if ((mkfifo(MYFIFO, 0666) < 0) && (errno != EEXIST))
        {
            printf("Cannot create fifo file\n");
            exit(1);
        }
    }

    /* 以只读阻塞方式打开有名管道 */
    fd = open(MYFIFO, O_RDONLY);
    if (fd == -1)
    {
        printf("Open fifo file error\n");
        exit(1);
    }

    memset(buff, 0, sizeof(buff));

    if ((nread = read(fd, buff, MAX_BUFFER_SIZE)) > 0)
    {
        printf("Read '%s' from FIFO\n", buff);
    }

   printf("***************** close fifo ************************\n");

    close(fd);

    exit(0);
}


void write_fifo(void)
{
    int fd;
    char buff[] = "this is a fifo test demo";
    int nwrite;

    sleep(2);   //等待子进程先运行

    /* 以只写阻塞方式打开 FIFO 管道 */
    fd = open(MYFIFO, O_WRONLY | O_CREAT, 0644);
    if (fd == -1)
    {
        printf("Open fifo file error\n");
        exit(1);
    }

    printf("Write '%s' to FIFO\n", buff);
    /*向管道中写入字符串*/
    nwrite = write(fd, buff, MAX_BUFFER_SIZE);
    
    if(wait(NULL))  //等待子进程退出
    {
        close(fd);
        exit(0);
    }

}


int main()
{
    pid_t result;
    /*调用 fork()函数*/
    result = fork();

    /*通过 result 的值来判断 fork()函数的返回情况，首先进行出错处理*/
    if(result == -1)
    {
        printf("Fork error\n");
    }


    else if (result == 0) /*返回值为 0 代表子进程*/
    {
        fifo_read();
    }

    else /*返回值大于 0 代表父进程*/
    {
        write_fifo();
    }

    return result;
}


