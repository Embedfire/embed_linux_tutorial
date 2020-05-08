/******* http客户端程序 httpclient.c ************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>

/********************************************
功能：搜索字符串右边起的第一个匹配字符
********************************************/
char *right_strchr(char * s, char x)
{
    int i = strlen(s);
    if (!(*s)) 
        return 0;
    while (s[i-1]) {
        if (strchr(s + (i - 1), x)) {
            return (s + (i - 1));
        } else {
            i--;
        }
    }
    return 0;
}

/********************************************
功能：把字符串转换为全小写
********************************************/
void str_to_lower(char * s)
{
    while (s && *s) {
        *s=tolower(*s);
        s++;
    }
}

/**************************************************************
功能：从字符串src中分析出网站地址和端口，并得到用户要下载的文件
***************************************************************/
void get_connect_info(char * src, char * host, char * file, int * port)
{
    char *p1;
    char *p2;

    memset(host, 0, sizeof(host));
    memset(file, 0, sizeof(file));

    *port = 0;

    if (!(*src)) 
        return;

    p1 = src;

    if (!strncmp(p1, "http://", strlen("http://"))) 
        p1 = src + strlen("http://");
    else if (!strncmp(p1, "https://", strlen("https://"))) 
        p1 = src + strlen("https://");
    
    p2 = strchr(p1, '/');

    if (p2) {
        memcpy(host, p1, strlen(p1) - strlen(p2));
        if (p2+1) {
            memcpy(file, p2 + 1, strlen(p2) - 1);
            file[strlen(p2) - 1] = 0;
        }
    } else {
        memcpy(host, p1, strlen(p1));
    }
    
    if (p2) 
        host[strlen(p1) - strlen(p2)] = 0;
    else 
        host[strlen(p1)] = 0;
    
    p1 = strchr(host, ':');

    if (p1) 
        *port = atoi(p1 + 1);
    else 
        *port = 80;     /* 只支持http协议 */
}


int main(int argc, char *argv[])
{
    int sockfd;
    char buffer[1024];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int port,nbytes;
    char host_addr[256];
    char host_file[1024];
    char local_file[256];
    FILE * fp;
    char request[1024];
    int send, totalsend;
    int i = 0;
    char * pt;

    if (argc!=2) {
        fprintf(stderr,"Usage:%s host-address\a\n",argv[0]);
        exit(1);
    }

    str_to_lower(argv[1]);/*将参数转换为全小写*/

    get_connect_info(argv[1], host_addr, host_file, &port);  /*分析网址、端口、文件名等*/

    // printf("host:%s\n", host_addr);
    // printf("hostfile:%s\n", host_file);
    // printf("port:%d\n\n", port);

    if ((host=gethostbyname(host_addr))==NULL) { /*取得主机IP地址*/
        fprintf(stderr,"Gethostname error, %s\n", strerror(errno));
        exit(1);
    }

    /* 客户程序开始建立 sockfd描述符 */
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) { /*建立SOCKET连接*/
        fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
        exit(1);
    }

    /* 客户程序填充服务端的资料 */
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr=*((struct in_addr *)host->h_addr);

    /* 客户程序发起连接请求 */
    if (connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1) { /*连接网站*/
        fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));
        exit(1);
    }

    /*准备 request 报文，将要发送给主机*/
    sprintf(request, "GET /%s HTTP/1.1\r\n"
                     "Accept: */*\r\n"
                     "Accept-Language: zh-cn\r\n"
                     "User-Agent: Mozilla/5.0\r\n"
                     "Host: %s:%d\r\n"
                     "Connection: Close\r\n\r\n", host_file, host_addr, port);

    /* 打印请求报文 */
    printf("%s", request);

    /*取得真实的文件名*/
    if (host_file && *host_file) 
        pt = right_strchr(host_file, '/');
    else 
        pt = 0;

    memset(local_file, 0, sizeof(local_file));

    if (pt && *pt) {
        if ((pt + 1) && *(pt+1)) 
            strcpy(local_file, pt + 1);
        else
            memcpy(local_file, host_file, strlen(host_file) - 1);

    } else if (host_file && *host_file) {
        strcpy(local_file, host_file);
        
    } else {
        strcpy(local_file, "index.html");
    }

    // printf("local filename to write:%s\n\n", local_file);

    /*发送http请求request*/
    send = 0;
    totalsend = 0;
    nbytes=strlen(request);

    while (totalsend < nbytes) {
        send = write(sockfd, request + totalsend, nbytes - totalsend);
        if (send==-1) {
            printf("send error!%s\n", strerror(errno));
            exit(0);
        }

        totalsend += send;
        // printf("%d bytes send OK!\n", totalsend);
    }

    fp = fopen(local_file, "w");
    if (!fp) {
        printf("create file error! %s\n", strerror(errno));
        return 0;
    }

    /* 连接成功了，接收http响应，response */
    while ((nbytes = read(sockfd, buffer, 1)) == 1) {

        /* 报文首部与主体之间有 \r\n\r\n */
        if (i < 4) {

            if (buffer[0] == '\r' || buffer[0] == '\n')
                i++;
            else 
                i = 0;
            /* 打印响应头部 */
            printf("%c", buffer[0]);
        } else {
            printf("%c", buffer[0]);
            fwrite(buffer, 1, 1, fp);/*将http主体信息写入文件*/
            i++;
            if (i%1024 == 0) 
                fflush(fp); /*每1K时存盘一次*/
        }
    }

    fclose(fp);
    /* 结束通讯 */
    close(sockfd);
    exit(0);
}


 