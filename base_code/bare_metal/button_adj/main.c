#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"


#include "button.h"
#include "led.h"




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

    /*初始化led灯和按键*/
    rgb_led_init();
    button2_init();

    while(1)
    {
        /*按键按下*/
        if(get_button2_status())
        {
            /*翻转绿灯状态*/
            if(i == 0)
            {
                green_led_on;
                i = 1;
            }
            else
            {
                green_led_off;
                i = 0;
            }
            while(get_button2_status());//等待按键松开
        }
    }

    return 0;    
}

