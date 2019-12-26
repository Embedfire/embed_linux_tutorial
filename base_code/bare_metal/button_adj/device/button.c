#include "button.h"



/*简单延时函数*/
void delay_button(uint32_t count)
{
    volatile uint32_t i = 0;
    for (i = 0; i < count; ++i)
    {
        __asm("NOP"); /* 调用nop空指令 */
    }
}



/*按键初始化函数*/
void button2_init(void)
{
    /*按键初始化*/
    CCM_CCGR1_CG15(0x3);  //开启GPIO5的时钟

    /*设置 绿灯 引脚的复用功能以及PAD属性*/
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01,0);     
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, button_PAD_CONFIG_DATA); 

    GPIO5->GDIR &= ~(1<<1);  //设置GPIO5_01为输入模式
}

/*按键状态输出函数*/
int get_button2_status(void)
{
    if((GPIO5->DR)&(1<<1))
    {
        delay_button(0xFF);
         if((GPIO5->DR)&(1<<1))
         {
             return 1;
         }
    }
    return 0;
}





