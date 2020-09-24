#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

char *led_on = "0\n";
char *led_off= "1\n";

int main(void)
{
    printf("led_cdev test\n");
    //打开设备
    int fd = open("/dev/led_chrdev0", O_RDWR);
    if(fd>0)
        printf("led_chrdev0 open success\n");
    else
        printf("led_chrdev0 open fail\n");
    //写入数据
    write(fd, led_on, strlen(led_on));
    //写入完毕，关闭文件
    close(fd);
    sleep(1);
    //打开设备
     fd = open("/dev/led_chrdev1", O_RDWR);
    if(fd>0)
        printf("led_chrdev1 open success\n");
    else
        printf("led_chrdev1 open fail\n");
    
    //写入数据
    write(fd, led_on, strlen(led_on));
    //写入完毕，关闭文件
    close(fd);
    sleep(1);
     //打开设备
     fd = open("/dev/led_chrdev2", O_RDWR);
    if(fd>0)
        printf("led_chrdev2 open success\n");
    else
        printf("led_chrdev2 open fail\n");
    //写入数据
    write(fd, led_on, strlen(led_on));
    //写入完毕，关闭文件
    close(fd);
    sleep(1);

    //关闭设备
    fd = open("/dev/led_chrdev0", O_RDWR);
    write(fd, led_off, strlen(led_off));
    close(fd);
     fd = open("/dev/led_chrdev1", O_RDWR);
    write(fd, led_off, strlen(led_off));
    close(fd);
     fd = open("/dev/led_chrdev2", O_RDWR);
    write(fd, led_off, strlen(led_off));
    close(fd);

    return 0;
}