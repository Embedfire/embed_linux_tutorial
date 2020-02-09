#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"
#include "system_MCIMX6Y2.h"

#include "button.h"
#include "led.h"



uint8_t button_status = 0;
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
    int i = 0;
    rgb_led_init();                          //初始化 RGB 灯，初始化后 默认所有灯都不亮。
    interrupt_button2_init();                //初始化引脚，和引脚的中断方式以及开启引脚中断。
    
    while (1)
    {
        if(button_status > 0)
        {
            /*绿灯亮*/
            red_led_off;
            green_led_on;

        }
        else
        {
            /*红灯亮*/
            green_led_off;
            red_led_on;
        }
        
    }

    return 0;
}

