#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int pid;
    sem_t *sem;
    const char sem_name[] = "my_sem_test";

    pid = fork();

    if (pid < 0) {
        printf("error in the fork!\n");
    } 
    /* 子进程 */
    else if (pid == 0) {
        /*创建/打开一个初始值为1的信号量*/
        sem = sem_open(sem_name, O_CREAT, 0644, 1);

        if (sem == SEM_FAILED) {
            printf("unable to create semaphore...\n");

            sem_unlink(sem_name);

            exit(-1);
        }
        /*获取信号量*/
        sem_wait(sem);

        for (int i = 0; i < 3; ++i) {

            printf("child process run: %d\n", i);
            /*睡眠释放CPU占用*/
            sleep(1);
        }

    /*释放信号量*/
    sem_post(sem);

    }
    /* 父进程 */
    else {

        /*创建/打开一个初始值为1的信号量*/
        sem = sem_open(sem_name, O_CREAT, 0644, 1);
        
        if (sem == SEM_FAILED) {
            printf("unable to create semaphore...\n");

            sem_unlink(sem_name);

            exit(-1);
        }
        /*申请信号量*/
        sem_wait(sem);

        for (int i = 0; i < 3; ++i) {

            printf("parent process run: %d\n", i);
            /*睡眠释放CPU占用*/
            sleep(1);
        }

        /*释放信号量*/
        sem_post(sem);
        /*等待子进程结束*/
        wait(NULL);

        /*关闭信号量*/
        sem_close(sem);
        /*删除信号量*/
        sem_unlink(sem_name);
    }

    return 0;
}
