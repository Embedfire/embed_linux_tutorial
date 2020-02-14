
/**
 * 函数原型:
 * int select(int numfds, fd_set *readfds, fd_set *writefds，
 *            fd_set *exeptfds, struct timeval *timeout) 
 * 
 * numfds：该参数值为需要监视的文件描述符的最大值加 1
 * readfds：由 select()监视的读文件描述符集合
 * writefds：由 select()监视的写文件描述符集合
 * exeptfds：由 select()监视的异常处理文件描述符集合
 * timeout 取值有3种：
 *      NULL：永远等待，直到捕捉到信号或文件描述符已准备好为止
 *      具体的等待时间值： struct timeval 类型的指针，若等待了 timeout 时间还没有检测到任何文件描符准备好，就立即返回
 *      0：从不等待，测试所有指定的描述符并立即返回
 * return : 大于 0：成功，返回准备好的文件描述符的数目
 *          0：超时
 *          -1：出错
 */

/********************* select()文件描述符处理函数 *****************************
 * 
 * FD_ZERO(fd_set *set)             清除一个文件描述符集
 * FD_SET(int fd, fd_set *set)      将一个文件描述符加入文件描述符集中
 * FD_CLR(int fd, fd_set *set)      将一个文件描述符从文件描述符集中清除
 * FD_ISSET(int fd, fd_set *set)    如果文件描述符 fd 为 fd_set 集中的一个元素，则返回非零值，
 * 可以用于调用 select()之后测试文件描述符集中的文件描述符是否有变化 
 * 
 * 一般来说，在使用 select()函数之前，首先使用 FD_ZERO()和 FD_SET()来初始化文件描述符集，在使用了
 * select()函数时，可循环使用 FD_ISSET()来测试描述符集，在执行完对相关文件描述符的操作之后，使用
 * FD_CLR()来清除描述符集。 
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024 /* 缓冲区大小*/
#define IN_FILES 3 /* 多路复用输入文件数目*/
#define TIME_DELAY 60 /* 超时值秒数 */
#define MAX(a, b) ((a > b)?(a):(b))


int main(void)
{
    int fds[IN_FILES];
    char buf[MAX_BUFFER_SIZE];
    int i, res, real_read, maxfd;

    struct timeval tv;
    fd_set inset,tmp_inset;

    /*首先以只读非阻塞方式打开两个管道文件*/
    fds[0] = 0;
    if((fds[1] = open ("in1", O_RDONLY|O_NONBLOCK)) < 0)
    {
        printf("Open in1 error\n");
        return 1;
    }

    if((fds[2] = open ("in2", O_RDONLY|O_NONBLOCK)) < 0)
    {
        printf("Open in2 error\n");
        return 1;
    }

    /*取出两个文件描述符中的较大者*/
    maxfd = MAX(MAX(fds[0], fds[1]), fds[2]);
    
    /*初始化读集合 inset，并在读集合中加入相应的描述集*/
    FD_ZERO(&inset);
    for (i = 0; i < IN_FILES; i++)
    {
        FD_SET(fds[i], &inset);
    }

    FD_SET(0, &inset);
    tv.tv_sec = TIME_DELAY;
    tv.tv_usec = 0;
    /*循环测试该文件描述符是否准备就绪，并调用 select 函数对相关文件描述符做对应操作*/
    while(FD_ISSET(fds[0],&inset)|| FD_ISSET(fds[1],&inset) || FD_ISSET(fds[2], &inset))
    {
        /* 文件描述符集合的备份， 这样可以避免每次进行初始化 */
        tmp_inset = inset;
        res = select(maxfd + 1, &tmp_inset, NULL, NULL, &tv);
        switch(res)
        {
            case -1:
            {
                printf("Select error\n");
                return 1;
            }
            break;

            case 0: /* Timeout */
            {
                printf("Time out\n");
                return 1;
            }
            break;

            default:
            {
                for (i = 0; i < IN_FILES; i++)
                {
                    if(FD_ISSET(fds[i], &tmp_inset))
                    {
                        memset(buf, 0, MAX_BUFFER_SIZE);
                        real_read = read(fds[i], buf, MAX_BUFFER_SIZE);
                        
                        if (real_read < 0)
                        {
                            if (errno != EAGAIN)
                            {
                                return 1;
                            }
                        }

                        else if (!real_read)
                        {
                            close(fds[i]);
                            FD_CLR(fds[i], &inset);
                        }
                        else
                        {
                            if (i == 0)
                            {/* 主程序终端控制 */
                                if ((buf[0] == 'q') || (buf[0] == 'Q'))
                                {
                                    return 1;
                                }
                            }
                        else
                        {
                            /* 显示管道输入字符串 */
                            buf[real_read] = '\0';
                            printf("%s", buf);
                        }
                        }
                    } /* end of if */
                } /* end of for */
            }
            break;
            
        } /* end of switch */
    } /*end of while */
    return 0;
}

