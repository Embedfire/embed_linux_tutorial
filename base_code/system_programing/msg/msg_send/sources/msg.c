/** msg.c */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define BUFFER_SIZE 512

struct message
{
    long msg_type;
    char msg_text[BUFFER_SIZE];
};
int main()
{
    int qid;
    struct message msg;

    /*创建消息队列*/
    if ((qid = msgget((key_t)1234, IPC_CREAT|0666)) == -1)
    {
        perror("msgget\n");
        exit(1);
    }

    printf("Open queue %d\n",qid);

    while(1)
    {
        printf("Enter some message to the queue:");
        if ((fgets(msg.msg_text, BUFFER_SIZE, stdin)) == NULL)
        {
            printf("\nGet message end.\n");
            exit(1);
        }  

        msg.msg_type = getpid();
        /*添加消息到消息队列*/
        if ((msgsnd(qid, &msg, strlen(msg.msg_text), 0)) < 0)
        {
            perror("\nSend message error.\n");
            exit(1);
        }
        else
        {
            printf("Send message.\n");
        }

        if (strncmp(msg.msg_text, "quit", 4) == 0)
        {
            printf("\nQuit get message.\n");
            break;
        }
    }

    exit(0);
}
