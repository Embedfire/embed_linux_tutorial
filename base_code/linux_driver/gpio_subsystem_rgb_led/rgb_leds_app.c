#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int main(int argc, char *argv[])
{

    printf("led_tiny test\n");
    /*判断输入的命令是否合法*/
    if(argc != 2)
    {
        printf(" commend error ! \n");
        return -1;
    }

    /*打开文件*/
    int fd = open("/dev/rgb-leds", O_RDWR);
    if(fd < 0)
    {
		printf("open file : %s failed !\n", argv[0]);
		return -1;
	}

    unsigned char commend = atoi(argv[1]);  //将受到的命令值转化为数字;

    /*判断命令的有效性*/

    /*写入命令*/
    int error = write(fd,&commend,sizeof(commend));
    if(error < 0)
    {
        printf("write file error! \n");
        close(fd);
        /*判断是否关闭成功*/
    }

    /*关闭文件*/
    error = close(fd);
    if(error < 0)
    {
        printf("close file error! \n");
    }
    
    return 0;
}