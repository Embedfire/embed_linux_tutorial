#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "test_app.h"
#include <sys/ioctl.h>

/*显示屏相关头文件*/
#include <linux/fb.h>
#include <sys/mman.h>

extern unsigned int test_picture[];

typedef struct lcd_color
{
    u8 bule;
    u8 green;
    u8 red;
    u8 alpha;
} lcd_color;


int main()
{
    int fp = 0;
    long screensize=0; 
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    char *fbp = 0;
    int x = 0, y = 0;
    long location = 0;


    fp = open("/dev/fb0", O_RDWR);

    if (fp < 0)
    {
        printf("Error : Can not open framebuffer device/n");
        exit(1);
    }

    if (ioctl(fp, FBIOGET_FSCREENINFO, &finfo))
    {
        printf("Error reading fixed information/n");
        exit(2);
    }

    if (ioctl(fp, FBIOGET_VSCREENINFO, &vinfo))
    {
        printf("Error reading variable information/n");
        exit(3);
    }
    // printf("The mem is :%d\n", finfo.smem_len);

    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
     /*这就是把fp所指的文件中从开始到screensize大小的内容给映射出来，得到一个指向这块空间的指针*/
    fbp =(char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp,0);
    if ((int) fbp == -1)
    {
       printf ("Error: failed to map framebuffer device to memory./n");
       exit (4);
    }

    /*刷红色*/
    int i = 0;
    lcd_color clear_color = {0,0,255,0};
    for(i=0; i < screensize; i+=4)
    {
        *((lcd_color*)(fbp + i)) = clear_color;
    }
    usleep(1000*2000);

    /*刷绿色*/
    clear_color.red = 0;
    clear_color.green = 255;
    clear_color.bule = 0;
    for(i=0; i < screensize; i+=4)
    {
        *((lcd_color*)(fbp + i)) = clear_color;
    }
    usleep(1000*2000);

    /*刷蓝色*/
    clear_color.red = 0;
    clear_color.green = 0;
    clear_color.bule = 255;
    for(i=0; i < screensize; i+=4)
    {
        *((lcd_color*)(fbp + i)) = clear_color;
    }

    /*显示图片*/
    memcpy(fbp,test_picture,800*480*4);


    munmap (fbp, screensize); /*解除映射*/
    printf("The mem is :%d\n", finfo.smem_len);
    printf("The line_length is :%d\n", finfo.line_length);
    printf("The xres is :%d\n", vinfo.xres);
    printf("The yres is :%d\n", vinfo.yres);
    printf("bits_per_pixel is :%d\n", vinfo.bits_per_pixel);
    close(fp);
    return 0;
}