/** stream_pipe.c */

/**
 * 与 Linux 的文件操作中有基于文件流的标准 I/O 操作一样，管道的操作也支持基于文件流的模式。这种基
 * 于文件流的管道主要是用来创建一个连接到另一个进程的管道，这里的"另一个进程"也就是一个可以进
 * 行一定操作的可执行文件，例如，用户执行"ls -l"或者自己编写的程序"./pipe"等。由于这一类操作很
 * 常用，因此标准流管道就将一系列的创建过程合并到一个函数 popen()中完成。它所完成的工作有以下几步:
 *   创建一个管道。
 *   fork()一个子进程。
 *   在父子进程中关闭不需要的文件描述符。
 *   执行 exec 函数族调用。
 *   执行函数中所指定的命令。
 * 
 * 这个函数的使用可以大大减少代码的编写量，但同时也有一些不利之处，例如，它不如前面管道创建的函
 * 数那样灵活多样，并且用 popen()创建的管道必须使用标准 I/O 函数进行操作，但不能使用前面的 read()、
 * write()一类不带缓冲的 I/O 函数。
 * 与之相对应，关闭用 popen()创建的流管道必须使用函数 pclose()来关闭该管道流。该函数关闭标准 I/O 流，
 * 并等待命令执行结束
 */

/**
 * 函数原型：FILE *popen(const char *command, const char *type)
 * 
 * 函数传入值：
 *  command：指向的是一个以 null 结束符结尾的字符串，这个字符串包含一个 shell 命令，并被送到/bin/sh 以-c 参数执行，即由 shell 来执行
 *  type：
 *      "r"：文件指针连接到 command 的标准输出，即该命令的结果产生输出
 *      "w"：文件指针连接到 command 的标准输入，即该命令的结果产生输入
 * 
 * 函数返回值：
 *  成功：文件流指针
 *  出错： -1
 */

/**
 * 函数原型 int pclose(FILE *stream)
 * 函数传入值 stream：要关闭的文件流
 * 函数返回值
 *  成功：返回由 popen()所执行的进程的退出码
 *  出错： -1
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#define BUFSIZE 1024
int main()
{
    FILE *fp;
    char *cmd = "ps -ef";
    char buf[BUFSIZE];
    
    /*调用 popen()函数执行相应的命令*/
    if ((fp = popen(cmd, "r")) == NULL)
    {
        printf("Popen error\n");
        exit(1);
    }

    while ((fgets(buf, BUFSIZE, fp)) != NULL)
    {
        printf("%s",buf);       //打印命令输出的信息
    }

    pclose(fp);
    
    exit(0);
}