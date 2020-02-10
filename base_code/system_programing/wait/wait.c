/** wait.c */
/**
 * pid_t waitpid(pid_t pid, int *status, int options)
 * 
 * pid：
 *  pid > 0：只等待进程 ID 等于 pid 的子进程，不管已经有其他子进程运行结束退出了，只要指定的子进程还没有结束， waitpid()就会一直等下去
 *  pid = -1：等待任何一个子进程退出，此时和 wait()作用一样
 *  pid = 0：等待其组 ID 等于调用进程的组 ID 的任一子进程
 *  pid < -1：等待其组 ID 等于 pid 的绝对值的任一子进程
 * 
 * status: 是一个整型指针，是该子进程退出时的状态， status 若不为空，则通过它可以获得子进程的结束状态
 * 另外，子进程的结束状态可由 Linux 中一些特定的宏来测定
 * 
 * options:
 *  WNOHANG：若由 pid 指定的子进程不立即可用，则 waitpid()不阻塞，此时返回值为 0
 *  WUNTRACED：若实现某支持作业控制，则由 pid 指定的任一子进程状态已暂停，且其状态自暂停以来还未报告过，则返回其状态
 *  0：同 wait()，阻塞父进程，等待子进程退出
 * 
 * 返回
 *  正常：已经结束运行的子进程的进程号
 *  使用选项 WNOHANG 且没有子进程退出： 0
 *  调用出错： -1
 */


/** 
 * demo 说明：
 * 使用 fork()创建一个子进程，然后让其子进程暂停 5s（使用了 sleep()函数）。接下来对原有的父进程使用 waitpid()函数，并使用参数
 * WNOHANG 使该父进程不会阻塞。若有子进程退出，则 waitpid()返回子进程号；若没有子进程退出，则waitpid()返回 0，并且父进程每隔一秒循环判断一次。
 */


#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define     WAIT   0
#define     WAITPID  1

#if ((WAIT | WAITPID) == 0)
#error "must choose a function to compile!"
#endif

#if (WAIT & WAITPID)
#error "have and can only choose one of the functions to compile"
#endif


#if WAIT

int main()
{
    pid_t pid, child_pid;
    int status;

    pid = fork();

    if (pid < 0) {
        printf("Error fork\n");
    }
    /*子进程*/
    else if (pid == 0) {

        printf("I am a child process!, my pid is %d!\n\n",getpid());

        /*子进程暂停 3s*/
        sleep(3);

        printf("I am about to quit the process!\n\n");

        /*子进程正常退出*/
        exit(0);
    }
    /*父进程*/
    else {

        /*调用 wait，父进程阻塞*/
        child_pid = wait(&status);

        /*若发现子进程退出，打印出相应情况*/
        if (child_pid == pid) {
            printf("Get exit child process id: %d\n",child_pid);
            printf("Get child exit status: %d\n\n",status);
        } else {
            printf("Some error occured.\n\n");
        }

        exit(0);
    }
}

#endif

#if WAITPID

int main()
{
    pid_t pid, child_pid;
    int status;

    pid = fork();

    if (pid < 0) {
        printf("Error fork\n");
    }
    /*子进程*/
    else if (pid == 0) {

        printf("I am a child process!, my pid is %d!\n\n",getpid());

        /*子进程暂停 3s*/
        sleep(3);

        printf("I am about to quit the process!\n\n");
        /*子进程正常退出*/
        exit(0);
    }
    /*父进程*/
    else {

        /*调用 waitpid，且父进程不阻塞*/
        child_pid = waitpid(pid, &status, WUNTRACED);

        /*若发现子进程退出，打印出相应情况*/
        if (child_pid == pid) {
            printf("Get exit child process id: %d\n",child_pid);
            printf("Get child exit status: %d\n\n",status);
        } else {
            printf("Some error occured.\n");
        }

        exit(0);
    }
}

#endif




