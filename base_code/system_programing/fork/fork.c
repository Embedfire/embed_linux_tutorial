/* fork.c */

/** 
 * fork()函数用于从已存在的进程中创建一个新进程。新进程称为子进程，而原进程称为父进程。使用fork()
 * 函数得到的子进程是父进程的一个复制品，它从父进程处继承了整个进程的地址空间，包括进程上下文、
 * 代码段、进程堆栈、内存信息、打开的文件描述符、信号控制设定、进程优先级、进程组号、当前工作目
 * 录、根目录、资源限制和控制终端等，而子进程所独有的只有它的进程号、资源使用和计时器等。 
 * 因为子进程几乎是父进程的完全复制，所以父子两个进程会运行同一个程序。
 * 
 * 在父进程中执行 fork()函数时，父进程会复制出一个子进程，而且父子进程的代码从fork()函数的
 * 返回开始分别在两个地址空间中同时运行。从而两个进程分别获得其所属fork()的返回值，其中在父进程
 * 中的返回值是子进程的进程号，而在子进程中返回 0 
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main(void)
{
    pid_t result;

    printf("This is a fork demo!\n\n");

    /*调用 fork()函数*/
    result = fork();

    /*通过 result 的值来判断 fork()函数的返回情况，首先进行出错处理*/
    if(result == -1) {
        printf("Fork error\n");
    }

    /*返回值为 0 代表子进程*/
    else if (result == 0) {
        printf("The returned value is %d, In child process!! My PID is %d\n\n", result, getpid());

    }

    /*返回值大于 0 代表父进程*/
    else {
        printf("The returned value is %d, In father process!! My PID is %d\n\n", result, getpid());
    }

    return result;
}





