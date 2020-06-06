#ifndef ECSPI_OLED_H
#define ECSPI_OLED_H
#include <linux/types.h>

//宏定义
//oled显示屏列数
#define X_WIDTH 		128  
//oled显示屏行数
#define Y_WIDTH 		64   


/*数据发送结构体*/
typedef struct oled_display_struct
{
	u8 x;
	u8 y;
    u32 length;
	u8 display_buffer[];
}oled_display_struct;

#endif