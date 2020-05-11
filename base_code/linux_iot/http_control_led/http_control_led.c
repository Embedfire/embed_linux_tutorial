#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h> // socket
#include <sys/types.h>  // 基本数据类型
#include <unistd.h> // read write
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> // open close
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/stat.h>


#define PORT        8888
#define SERVER      "0.0.0.0"
#define BACKLOG     20
#define BUFF_SIZE   (1024 * 500)

#define LED_ON      "./led_on.html"
#define LED_OFF     "./led_off.html"

int sockfd;
char *http_res_tmpl = "HTTP/1.1 200 OK\r\n"
        "Server: Cleey's Server V1.0\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Content-Type: %s\r\n\r\n";
  
int http_str_nmatch(const char *str1, const char *str2, int n)
{
    int i = 0;
    int c1, c2;
    do {
        c1 = *str1++;
        c2 = *str2++;
        i++;
    } while(((c1 == c2) && c1) && (i < n));

    return c1 - c2;
}

void http_send(int sock_client, char *str) 
{
    char header[BUFF_SIZE], body[BUFF_SIZE];

    int len = strlen(str);

    sprintf(header, http_res_tmpl, len,"text/html");

    len = sprintf(body,"%s%s", header, str);

    send(sock_client, body, len, 0);
}

void handle_signal(int sign) 
{
    fputs("\nSIGNAL INTERRUPT \nBye Cleey! \nSAFE EXIT\n",stdout);
    close(sockfd);
    exit(0);
}


int read_file(char *filename, int *len, char **data)
{
    int file = open(filename, O_RDONLY);
    if ( file == -1 )
        return -1;

    int i = 0;
    while ( 1 )
    {
        *data = realloc(*data, (BUFF_SIZE * (i + 1)));
        if ( data == NULL )
        {
            close( file );
            return -1;
        }

        int cur_len = read(file, *data+(BUFF_SIZE * i), BUFF_SIZE);
        if ( cur_len == 0 )
            break;
        else
            *len += cur_len;

        i++;
    }

    close( file );

    return 0;
}

int main(void)
{
    signal(SIGINT,handle_signal);
    int len = 0;
    char *pdata = NULL;
    int count = 0; // 计数
    // 定义 socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    // 定义 sockaddr_in
    struct sockaddr_in skaddr;
    skaddr.sin_family = AF_INET;            // ipv4
    skaddr.sin_port   = htons(PORT);
    skaddr.sin_addr.s_addr = inet_addr(SERVER);
    // bind，绑定 socket 和 sockaddr_in
    if (bind(sockfd,(struct sockaddr *)&skaddr,sizeof(skaddr)) == -1 ) {
        perror("bind error");
        exit(1);
    }

    // listen，开始添加端口
    if (listen(sockfd, BACKLOG) == -1 ) {
        perror("listen error");
        exit(1);
    }

    // 客户端信息
    char buff[BUFF_SIZE];
    struct sockaddr_in claddr;
    socklen_t length = sizeof(claddr);

    while(1) {
        int sock_client = accept(sockfd,(struct sockaddr *)&claddr, &length);
        if (sock_client <0) {
            perror("accept error");
            exit(1);
        }

        memset(buff,0,sizeof(buff));

        int len = recv(sock_client, buff, sizeof(buff), 0);

        if (http_str_nmatch(buff, "GET /", 5) == 0) {

            read_file(LED_ON, &len, &pdata);
            http_send(sock_client, pdata);

        } else if (http_str_nmatch(buff, "POST /on", 8) == 0) {

            read_file(LED_ON, &len, &pdata);
            http_send(sock_client, pdata);

            printf("led on\n");

        } else if (http_str_nmatch(buff, "POST /off", 8) == 0) {

            read_file(LED_OFF, &len, &pdata);
            http_send(sock_client, pdata);

            printf("led off\n");
        } else {
            http_send(sock_client,"Hello World!");
        }

        // fputs(buff,stdout);
        close(sock_client);
    }
    fputs("Bye Cleey",stdout);
    close(sockfd);
    return 0;
}
