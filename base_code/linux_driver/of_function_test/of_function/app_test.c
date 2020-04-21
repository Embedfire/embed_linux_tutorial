#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int main(void)
{
    printf("led_tiny test\n");
 
    //打开文件
    int fd = open("/dev/of_test", O_RDWR);

    //关闭文件
    close(fd);
    printf("tiny led test finished !\n");
    return 0;
}