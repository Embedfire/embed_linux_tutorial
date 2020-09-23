#include "stdio.h"
#include "sys/types.h"
#include "sys/ioctl.h"
#include "stdlib.h"
#include "termios.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "sys/time.h"
#include <sys/select.h>

main()
{
    int fd;
    char buf[20]={0};
    float result;
    int readlen = 0;

    if ((fd=open("/dev/ds18b20",O_RDWR | O_NDELAY | O_NOCTTY)) < 0) {
        printf("Open Device Ds18b20 Failed.\r\n");
        exit(1);
    }
    else
    {
        printf("Open Device Ds18b20 Successed.\r\n");
        while(1) {
            readlen = read(fd, buf, sizeof(buf));
            // printf("%d.%d C\r\n", buf[0], buf[1]);
            printf("%s", buf);
            usleep(20000);
        }
        close(fd);
    }
}

   