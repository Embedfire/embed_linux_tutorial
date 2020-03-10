#ifndef _SEM_H_
#define _SEM_H_


union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int init_sem(int sem_id, int init_value);
int del_sem(int sem_id);
int sem_p(int sem_id);
int sem_v(int sem_id);




#endif

