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
    } else if (pid == 0) {
        
        sem = sem_open("my_sem_test", O_CREAT, 0644, 1);

        if (sem == SEM_FAILED) {
            printf("unable to create semaphore...\n");

            sem_unlink(sem_name);

            exit(-1);
        }

        sem_wait(sem);

        for (int i = 0; i < 3; ++i) {

            printf("childe process run: %d\n", i);
            sleep(1);
        }

    sem_post(sem);

    } else {

        sem = sem_open(sem_name, O_CREAT, 0644, 1);
        
        if (sem == SEM_FAILED) {
            printf("unable to create semaphore...\n");

            sem_unlink(sem_name);

            exit(-1);
        }

        sem_wait(sem);

        for (int i = 0; i < 3; ++i) {

            printf("parent process run: %d\n", i);
            sleep(1);
        }

        sem_post(sem);

        wait(NULL);
    }

    sem_close(sem);
    sem_unlink(sem_name);

    return 0;
}
