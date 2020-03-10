
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "sem.h"

#define DELAY_TIME 3 /* 为了突出演示效果，等待几秒钟， */

int main(void)
{
    pid_t result;
    int sem_id;

    sem_id = semget((key_t)6666, 1, 0666 | IPC_CREAT); /* 创建一个信号量*/

    init_sem(sem_id, 0);

    /*调用 fork()函数*/
    result = fork();
    if(result == -1)
    {
        perror("Fork\n");
    }
    else if (result == 0) /*返回值为 0 代表子进程*/
    {
        printf("Child process will wait for some seconds...\n");
        sleep(DELAY_TIME);
        printf("The returned value is %d in the child process(PID = %d)\n",result, getpid());

        sem_v(sem_id);
    }

    else /*返回值大于 0 代表父进程*/
    {
        sem_p(sem_id);
        printf("The returned value is %d in the father process(PID = %d)\n",result, getpid());

        sem_v(sem_id);

        del_sem(sem_id);
    }

    exit(0);
}
