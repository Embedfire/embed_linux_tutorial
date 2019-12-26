#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"


#include "led.h"






/*led初始化函数*/
void rgb_led_init(void)
{
        /*led初始化*/
    CCM_CCGR1_CG13(0x3);//开启GPIO1的时钟
    CCM_CCGR3_CG6(0x3); //开启GPIO4的时钟

        /*设置 红灯 引脚的复用功能以及PAD属性*/
    IOMUXC_SetPinMux(RGB_RED_LED_IOMUXC,0);     
    IOMUXC_SetPinConfig(RGB_RED_LED_IOMUXC, LED_PAD_CONFIG_DATA); 

    /*设置 绿灯 引脚的复用功能以及PAD属性*/
    IOMUXC_SetPinMux(RGB_GREEN_LED_IOMUXC,0);     
    IOMUXC_SetPinConfig(RGB_GREEN_LED_IOMUXC, LED_PAD_CONFIG_DATA); 

    /*设置 蓝灯 引脚的复用功能以及PAD属性*/
    IOMUXC_SetPinMux(RGB_BLUE_LED_IOMUXC,0);     
    IOMUXC_SetPinConfig(RGB_BLUE_LED_IOMUXC, LED_PAD_CONFIG_DATA); 


    GPIO1->GDIR |= (1<<4);  //设置GPIO1_04为输出模式
    GPIO1->DR |= (1<<4);    //设置GPIO1_04输出电平为高电平

    GPIO4->GDIR |= (1<<20);  //设置GPIO4_20为输出模式
    GPIO4->DR |= (1<<20);    //设置GPIO4_20输出电平为高电平

    GPIO4->GDIR |= (1<<19);  //设置GPIO4_19为输出模式
    GPIO4->DR |= (1<<19);    //设置GPIO4_19输出电平为高电平
}







