#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"


/*LED GPIO端口、引脚号及IOMUXC复用宏定义*/
#define RGB_RED_LED_GPIO                GPIO1
#define RGB_RED_LED_GPIO_PIN            (4U)
#define RGB_RED_LED_IOMUXC              IOMUXC_GPIO1_IO04_GPIO1_IO04

#define RGB_GREEN_LED_GPIO              GPIO4
#define RGB_GREEN_LED_GPIO_PIN          (20U)
#define RGB_GREEN_LED_IOMUXC            IOMUXC_CSI_HSYNC_GPIO4_IO20

#define RGB_BLUE_LED_GPIO               GPIO4
#define RGB_BLUE_LED_GPIO_PIN           (19U)
#define RGB_BLUE_LED_IOMUXC             IOMUXC_CSI_VSYNC_GPIO4_IO19



/* 所有引脚均使用同样的PAD配置 */
#define LED_PAD_CONFIG_DATA            (SRE_0_SLOW_SLEW_RATE| \
                                        DSE_6_R0_6| \
                                        SPEED_2_MEDIUM_100MHz| \
                                        ODE_0_OPEN_DRAIN_DISABLED| \
                                        PKE_0_PULL_KEEPER_DISABLED| \
                                        PUE_0_KEEPER_SELECTED| \
                                        PUS_0_100K_OHM_PULL_DOWN| \
                                        HYS_0_HYSTERESIS_DISABLED)   
    /* 配置说明 : */
    /* 转换速率: 转换速率慢
      驱动强度: R0/6 
      带宽配置 : medium(100MHz)
      开漏配置: 关闭 
      拉/保持器配置: 关闭
      拉/保持器选择: 保持器（上面已关闭，配置无效）
      上拉/下拉选择: 100K欧姆下拉（上面已关闭，配置无效）
      滞回器配置: 关闭 */  


/*简单延时函数*/
void delay(uint32_t count)
{
    volatile uint32_t i = 0;
    for (i = 0; i < count; ++i)
    {
        __asm("NOP"); /* 调用nop空指令 */
    }
}

int main()
{
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

    while(1)
    {
         GPIO1->DR &= ~(1<<4); //红灯亮
         delay(0xFFFFF);
         GPIO1->DR |= (1<<4); //红灯灭

         GPIO4->DR &= ~(1<<20); //绿灯亮
         delay(0xFFFFF);
         GPIO4->DR |= (1<<20); //绿灯灭

         GPIO4->DR &= ~(1<<19); //蓝灯亮
         delay(0xFFFFF);
         GPIO4->DR |= (1<<19); //蓝灯灭
    }

    return 0;    
}




