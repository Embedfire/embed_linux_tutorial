#ifndef _BSP_BEEP_H_
#define _BSP_BEEP_H_


//蜂鸣器的GPIO引脚号
//imx6的计算方式，GPIOn_IOx = (n-1)*32 + x
//如GPIO1_IO19 = (1-1)*32 + 19 = 19
#define BEEP_GPIO_INDEX 	"19"

/**
 * @brief  初始化蜂鸣器gpio相关
 */
extern void beep_init(void);

/**
 * @brief  关闭蜂鸣器gpio的export输出
 */
extern void beep_deinit(void);

extern void beep_on(void);
extern void beep_off(void);


#endif

