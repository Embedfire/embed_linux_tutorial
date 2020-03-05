#ifndef _SEM_H_
#define _SEM_H_


union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

extern int init_sem(int sem_id, int init_value);
extern int del_sem(int sem_id);
extern int sem_p(int sem_id);
extern int sem_v(int sem_id);

#endif

