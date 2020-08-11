#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "test_app.h"

#include <linux/input.h>

struct input_event button_input_event;
int button_status = 0;   //保存按键状态;

int main(int argc, char *argv[])
{
    int error = -20;
    int button_status = 0;

    /*打开文件*/
    int fd = open("/dev/input/event1", O_RDONLY);
    if (fd < 0)
    {
        printf("open file : /dev/input/event1 error!\n");
        return -1;
    }

    printf("wait button down... \n");
    printf("wait button down... \n");
    
    do
    {
        /*读取按键状态*/
        error = read(fd, &button_input_event, sizeof(button_input_event));
        if (error < 0)
        {
            printf("read file error! \n");
        }
        if((button_input_event.type == 1) && (button_input_event.code == 2))
        {
            if(button_input_event.value == 0)
            {
                printf("button up\n");
            }
            else if(button_input_event.value == 1)
            {
                 printf("button down\n");
            }
        }
    } while (1);

    printf("button Down !\n");

    /*关闭文件*/
    error = close(fd);
    if (error < 0)
    {
        printf("close file error! \n");
    }
    return 0;
}
