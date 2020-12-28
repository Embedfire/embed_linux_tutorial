/** kill.c */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;

    int ret;

    /* 创建一子进程 */
    if ((pid = fork()) < 0) {
        printf("Fork error\n");
        exit(1);
    }

    if (pid == 0) {
        /* 在子进程中使用 raise()函数发出 SIGSTOP 信号,使子进程暂停 */
        printf("Child(pid : %d) is waiting for any signal\n\n", getpid());

        /** 子进程停在这里 */
        raise(SIGSTOP);

        exit(0);
    }

    else {
        /** 等待一下，等子进程先执行 */
        sleep(1);

        /* 在父进程中收集子进程发出的信号(不阻塞)，并调用 kill()函数进行相应的操作 */
        if ((waitpid(pid, NULL, WNOHANG)) == 0) {
            /** 子进程还没退出，返回为0，就发送SIGKILL信号杀死子进程 */
            if ((ret = kill(pid, SIGKILL)) == 0) {
                printf("Parent kill %d\n\n",pid);
            }
        }

        /** 一直阻塞直到子进程退出（杀死） */
        waitpid(pid, NULL, 0);

        exit(0);
    }
}
