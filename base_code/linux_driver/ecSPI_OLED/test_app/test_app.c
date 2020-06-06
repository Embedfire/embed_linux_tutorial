#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "test_app.h"

/*点阵数据*/
extern unsigned char F16x16[];
extern unsigned char F6x8[][6];
extern unsigned char F8x16[][16];
extern unsigned char BMP1[];

int main(int argc, char *argv[])
{
    int error = -1;
    /*打开文件*/
    int fd = open("/dev/ecspi_oled", O_RDWR);
    if (fd < 0)
    {
        printf("open file : %s failed !\n", argv[0]);
        return -1;
    }

    /*显示图片*/
    show_bmp(fd, 0, 0, BMP1, X_WIDTH*Y_WIDTH/8);
    sleep(3);

    /*显示文字*/
    oled_fill(fd, 0, 0, 127, 7, 0x00);  //清屏
    oled_show_F16X16_letter(fd,0, 0, F16x16, 4);  //显示汉字
    oled_show_F8X16_string(fd,0,2,"F8X16:THIS IS ecSPI TEST APP"); 
    oled_show_F6X8_string(fd, 0, 6,"F6X8:THIS IS ecSPI TEST APP");
    sleep(3);
    
    oled_fill(fd, 0, 0, 127, 7, 0x00);  //清屏
    oled_show_F8X16_string(fd,0,0,"Testing is completed"); 
    sleep(3);

    /*关闭文件*/
    error = close(fd);
    if(error < 0)
    {
        printf("close file error! \n");
    }

    return 0;
}




/*显示图片
*fd,打开文件描述符
*x, 图片显示起始位置x坐标，常规图片要将x设置为0，或者将图片逐行显示，
*y, 图片显示起始位置y坐标
*buffer，要显示的数据地址
*length，数据长度
*/
int show_bmp(int fd, u8 x, u8 y, u8 *buffer, u32 length)
{
    int error;
    struct oled_display_struct *display_struct;

    /*申请内存*/
    display_struct = malloc(sizeof(oled_display_struct) + length);

    /*填充发送结构体*/
    display_struct->x = x;
    display_struct->y = y;
    display_struct->length = length;
    memcpy(display_struct->display_buffer, buffer, display_struct->length);

    /*执行写入*/
    error = write(fd, display_struct, sizeof(oled_display_struct) + display_struct->length);

    /*释放内存*/
    free(display_struct);
    return error;
}



/*显示色块(清屏或刷屏)
*fd,打开文件描述符
*x,y, 起始坐标
*end_x,end_y 结束坐标
*data, 要显示的数据(0x00 黑屏，0xFF全亮。其他...)
*/
int oled_fill(int fd, u8 start_x, u8 start_y, u8 end_x, u8 end_y, u8 data)
{
    struct oled_display_struct *display_struct = NULL;
    int error;

    if ((end_x < start_x) || (end_y < start_y))
    {
        return -1;
    }

    /*填充要发送的数据*/
    display_struct = malloc(sizeof(oled_display_struct) + end_x - start_x + 1);
    if (display_struct == NULL)
    {
        printf("oled_fill malloc error \n");
        return -1;
    }
    display_struct->length = end_x - start_x + 1;
    memset(display_struct->display_buffer, data, display_struct->length);

    do
    {
        /*设置写入位置*/
        display_struct->x = start_x;
        display_struct->y = start_y;
        /*执行写入*/
        write(fd, display_struct, sizeof(oled_display_struct) + display_struct->length);
        start_y++;
    } while (start_y <= end_y);

    /*释放内存*/
    free(display_struct);
    return 0;
}

/*向 oled 写入一个字符
*fd,打开文件描述符
*x,y, 起始坐标
*width, 字符宽度
*high, 字符高度
*data,字符数据地址
*/
int oled_show_one_letter(int fd, u8 x, u8 y, u8 width, u8 high, u8 *data)
{
    struct oled_display_struct *display_struct = NULL;

    if ((high % 8) != 0)
    {
        printf("oled_show_one_letter  \"high\" set error! \n");
        return -1;
    }
    high = high/8;

    /*申请空间*/
    display_struct = malloc(sizeof(oled_display_struct) + width * high);
    if (display_struct == NULL)
    {
        printf("oled_fill malloc error \n");
        return -1;
    }

    do
    {
        display_struct->x = x;
        display_struct->y = y;
        display_struct->length = width;
        memcpy(display_struct->display_buffer, data, display_struct->length);
        write(fd, display_struct, sizeof(oled_display_struct) + display_struct->length);
        data += display_struct->length;
        high--;
        y++;
    } while (high > 0);

    /*释放空间*/
    free(display_struct);
    return 0;
}


/*写入F16X16字符
*fd,打开文件描述符
*x,y, 起始坐标
*data,字符数据地址
*length,字符个数
*/
int oled_show_F16X16_letter(int fd, u8 x, u8 y, u8 *data, u32 length)
{
    /*判断数据是否有效*/
    if ((x > (127 - 16)) || (y > (7 - 2)))
    {
        printf("oled_show_F16X16_letter \"x\" or \"y\"  set error! \n");
        return -1;
    }

    do
    {
        oled_show_one_letter(fd, x, y, 16, 16, data);
        /* 32 = 16(width) * 16(high) / 8(一个字节8位)*/
        data += 32;
        length--;
        x += 16;
        if (x > 127-16)
        {
            /*清除未写入的区域*/
            /*清除未写入的区域*/
            // oled_fill(fd, x, y, 127, y+1, 0x00);
            x = 0;
            y += 2;
        }
    } while (length>0);
    return 0;
}


/*写入F6x8字符串
*fd,打开文件描述符
*x,y, 起始坐标
*data,字符数据地址
*length,字符个数
*/
int oled_show_F6X8_string(int fd, u8 x, u8 y, u8 *string)
{
    u32 i = 0;
    u8 value = 0;
    /*判断数据是否有效*/
    if ((x > (127 - 6)) || (y > 7)||(string[0] == '\0'))
    {
        printf("oled_show_F16X16_letter \"x\" or \"y\"  set error! \n");
        return -1;
    }

    do
    {
        value = string[i] - 32;
        oled_show_one_letter(fd, x, y, 6, 8, F6x8[value]);
        /* 32 = 16(width) * 16(high) / 8(一个字节8位)*/
        x += 6;
        i++;
        if (x > (127-6))
        {
            /*清除未写入的区域*/
            // oled_fill(fd, x, y, 127, y, 0x00);
            x = 0;
            y++;
        }
    } while ((y<=7)&&(string[i] != '\0'));
    return 0;
}


/*写入F8X16字符串
*fd,打开文件描述符
*x,y, 起始坐标
*data,字符数据地址
*length,字符个数
*/
int oled_show_F8X16_string(int fd, u8 x, u8 y, u8 *string)
{
    u32 i = 0;
    u8 value = 0;
    /*判断数据是否有效*/
    if ((x > (127 - 8)) || (y > (7 - 1))||(string[0] == '\0'))
    {
        printf("oled_show_F16X16_letter \"x\" or \"y\"  set error! \n");
        return -1;
    }

    do
    {
        value = string[i] - 32;
        oled_show_one_letter(fd, x, y, 8, 16, F8x16[value]);
        /* 32 = 16(width) * 16(high) / 8(一个字节8位)*/
        x += 8;
        i++;
        if (x > (127-8))
        {
            /*清除未写入的区域*/
            // oled_fill(fd, x, y, 127, y, 0x00);
            x = 0;
            y += 2;
        }
    } while ((y<=6)&&(string[i] != '\0'));
    return 0;
}

