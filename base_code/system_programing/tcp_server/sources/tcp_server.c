#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX 10*1024
#define PORT 6666

// Driver function
int main()
{
    char buff[MAX];
    int n;
    int sockfd, connfd, len;
    struct sockaddr_in server, client;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    printf("socket successfully created..\n");
    bzero(&server, sizeof(server));

    // assign IP, PORT
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    // binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&server, sizeof(server))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    
    printf("socket successfully binded..\n");

    // now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }

    printf("server listening...\n");
    
    len = sizeof(client);

    // accept the data packet from client and verification
    connfd = accept(sockfd, (struct sockaddr*)&client, &len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    }

    printf("server acccept the client...\n");

    // infinite loop for chat
    while(1) {
        bzero(buff, MAX);

        // read the messtruct sockaddrge from client and copy it in buffer
        if (read(connfd, buff, sizeof(buff)) <= 0) {
            printf("client close...\n");
            close(connfd);
            break;
        }

        // print buffer which contains the client contents
        printf("from client: %s\n", buff);

        // if msg contains "Exit" then server exit and chat ended.
        if (strncmp("exit", buff, 4) == 0) {
            printf("server exit...\n");
            close(connfd);
            break;
        }
    }

    // After chatting close the socket
    close(sockfd);
    exit(0);
}

