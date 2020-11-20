/* system.c */

/** 
 * 这个 system()函数是C 标准库中提供的，它主要是提供了一种调用其它程序的简单方法。读者可以利用 system()函数调用一些应用程序，
 * 它产生的结果与从 shell 中执行这个程序基本相似。事实上， system()启动了一个运行着/bin/sh的子进程，然后将命令交由它执行。
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    pid_t result;

    printf("This is a system demo!\n\n");

    /*调用 system()函数*/
    result = system("ls -l &");

    printf("Done!\n\n");

    return result;
}





