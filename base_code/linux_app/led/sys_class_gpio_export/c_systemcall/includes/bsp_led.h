#ifndef _BSP_LED_H_
#define _BSP_LED_H_
//system函数的头文件
#include <stdlib.h>
/** 控制LED灯亮灭的宏，
  * LED低电平亮，设置ON=0，OFF=1
  * 若LED高电平亮，把宏设置成ON=1 ，OFF=0 即可
  */
#define ON  0
#define OFF 1
/* 带参宏，可以像内联函数一样使用 */
#define LED1(a)	    if (a)	\
		system("echo 1 >/sys/class/gpio/gpio4/value");\
	else		\
		system("echo 0 >/sys/class/gpio/gpio4/value");

#define LED2(a)	    if (a)	\
		system("echo 1 >/sys/class/gpio/gpio116/value");\
	else		\
		system("echo 0 >/sys/class/gpio/gpio116/value");

#define LED3(a)	    if (a)	\
		system("echo 1 >/sys/class/gpio/gpio115/value");\
	else		\
		system("echo 0 >/sys/class/gpio/gpio115/value");                    

/* 基本混色，后面高级用法使用PWM可混出全彩颜色,且效果更好 */

//红
#define LED_RED  \
		LED1(ON);\
		LED2(OFF);\
		LED3(OFF)

//绿
#define LED_GREEN		\
		LED1(OFF);\
		LED2(ON);\
		LED3(OFF)

//蓝
#define LED_BLUE	\
		LED1(OFF);\
		LED2(OFF);\
		LED3(ON)

					
//黄(红+绿)					
#define LED_YELLOW	\
		LED1(ON);\
		LED2(ON);\
		LED3(OFF)

//紫(红+蓝)
#define LED_PURPLE	\
		LED1(ON);\
		LED2(OFF);\
		LED3(ON)

//青(绿+蓝)
#define LED_CYAN \
		LED1(OFF);\
		LED2(ON);\
		LED3(ON)
					
//白(红+绿+蓝)
#define LED_WHITE	\
		LED1(ON);\
		LED2(ON);\
		LED3(ON)

//黑(全部关闭)
#define LED_RGBOFF	\
		LED1(OFF);\
		LED2(OFF);\
		LED3(OFF)		

extern void LED_GPIO_Config(void);
extern void LED_GPIO_DeInit(void);
#endif /* _BSP_LED_H_ */



