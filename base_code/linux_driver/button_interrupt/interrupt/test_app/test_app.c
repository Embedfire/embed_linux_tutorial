#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "test_app.h"

int main(int argc, char *argv[])
{
    int error = -20;
    int button_status = 0;

    /*打开文件*/
    int fd = open("/dev/button", O_RDWR);
    if (fd < 0)
    {
        printf("open file : /dev/button error!\n");
        return -1;
    }

    printf("wait button down... \n");
    printf("wait button down... \n");

    do
    {
        /*读取按键状态*/
        error = read(fd, &button_status, sizeof(button_status));
        if (error < 0)
        {
            printf("read file error! \n");
        }
        usleep(1000 * 100); //延时100毫秒
    } while (0 == button_status);
    printf("button Down !\n");

    /*关闭文件*/
    error = close(fd);
    if (error < 0)
    {
        printf("close file error! \n");
    }
    return 0;
}
