#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
char *wbuf = "Hello World\n";
char rbuf[128];
int main(void)
{
    printf("EmbedCharDev test\n");
    //打开文件
    int fd = open("/dev/chrdev", O_RDWR);
    //写入数据
    write(fd, wbuf, strlen(wbuf));
    //写入完毕，关闭文件
    close(fd);
    //打开文件
     fd = open("/dev/chrdev", O_RDWR);
    //读取文件内容
    read(fd, rbuf, 128);
    //打印读取的内容
    printf("The content : %s", rbuf);
    //读取完毕，关闭文件
    close(fd);
    return 0;
}