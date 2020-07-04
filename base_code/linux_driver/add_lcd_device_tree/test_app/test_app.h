
#ifndef TEST_APP_H
#define TEST_APP_H


//宏定义
#define X_WIDTH 		128  //oled显示屏列数
#define Y_WIDTH 		64   //oled显示屏行数

#define u8 unsigned char
#define u32 unsigned int


/*数据发送结构体*/
typedef struct oled_display_struct
{
	u8 x;
	u8 y;
    u32 length;
	u8 display_buffer[];
}oled_display_struct;
 
int show_bmp(int fd, u8 x, u8 y,u8* buffer, u32 length);
int oled_fill(int fd, u8 start_x, u8 start_y,u8 end_x, u8 end_y,u8 data);
int oled_show_one_letter(int fd, u8 x, u8 y, u8 width, u8 high, u8 *data);
int oled_show_F16X16_letter(int fd, u8 x, u8 y, u8 *data, u32 length);
int oled_show_F6X8_string(int fd, u8 x, u8 y, u8 *string);
int oled_show_F8X16_string(int fd, u8 x, u8 y, u8 *string);

#endif