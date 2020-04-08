#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/*多线程需要用到的头文件*/
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{

    pid_t pid; //用于保存 fork 函数返回的父、子线程的PID
    int fd;

    /*判断输入的命令是否合法*/
    if (argc != 2)
    {
        printf(" commend error ! \n");
        return -1;
    }

    pid = fork();
    if (pid < 0)
    {
        /*fork 函数执行错误*/
        printf("\n fork error ！！\n");
        return -1;
    }

    if (0 == pid)
    {
        printf("\n child! \n");
        /*这里是子线程*/
        fd = open("/dev/led_chrdev0", O_RDWR); //打开设 "led_chrdev0"
        if (fd < 0)
        {
            printf("\n open file : /dev/led_chrdev0 failed !!!\n");
            return -1;
        }

        /*写入命令*/
        int error = write(fd, argv[1], sizeof(argv[1]));
        if (error < 0)
        {
            printf("write file error! \n");
            close(fd);
            /*判断是否关闭成功*/
        }

        /*关闭文件*/
        error = close(fd);
        if (error < 0)
        {
            printf("close file error! \n");
        }
    }
    else
    {
        printf("\n parent! \n");
        /*这里是父进程*/
        fd = open("/dev/led_chrdev1", O_RDWR); //打开设 "led_chrdev1"
        if (fd < 0)
        {
            printf("\n open file : /dev/led_chrdev1 failed !!!\n");
            return -1;
        }

        /*写入命令*/
        int error = write(fd, argv[1], sizeof(argv[1]));
        if (error < 0)
        {
            printf("write file error! \n");
            close(fd);
            /*判断是否关闭成功*/
        }

        /*关闭文件*/
        error = close(fd);
        if (error < 0)
        {
            printf("close file error! \n");
        }
    }

    return 0;
}