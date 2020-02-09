#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"
#include "system_MCIMX6Y2.h"

#include "button.h"
#include "led.h"
#include "clock.h"
#include "uart.h"

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

/*提示字符串*/
uint8_t txbuff[] = "Uart polling example\r\nBoard will send back received characters\r\n";
int main()
{
    uint8_t ch; //用于暂存串口收到的字符

    system_clock_init();
    rgb_led_init();           //初始化 RGB 灯，初始化后 默认所有灯都不亮。
    interrupt_button2_init(); //初始化引脚，和引脚的中断方式以及开启引脚中断。
    uart_init();
    UART_WriteBlocking(UART1, txbuff, sizeof(txbuff) - 1);

    while (1)
    {
        UART_ReadBlocking(UART1, &ch, 1);
        UART_WriteBlocking(UART1, &ch, 1);
    }

    return 0;
}
