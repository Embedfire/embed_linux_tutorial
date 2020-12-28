#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"
#include "system_MCIMX6Y2.h"

#include "button.h"
#include "led.h"
#include "clock.h"
#include "uart.h"

#include "elcdif.h"

uint8_t button_status = 0;  // 按键状态
static volatile unsigned char s_frameDone = false;  // elcdf 帧传输状态


/*提示字符串*/
uint8_t txbuff[] = "Uart polling example\r\nBoard will send back received characters\r\n";


/*简单延时函数*/
void delay(uint32_t count)
{
    volatile uint32_t i = 0;
    for (i = 0; i < count; ++i)
    {
        __asm("NOP"); /* 调用nop空指令 */
    }
}




/*
* elcdf 帧传输完成中断
*/
void APP_LCDIF_IRQHandler(void)
{
    uint32_t intStatus = 0;

    /*获取传输完成中断的状态，*/
    intStatus = ((LCDIF->CTRL1) & (1 <<9));
     /*清除 1 帧传输完成中断标志位*/
    LCDIF->CTRL1_CLR = (1 << 9);

    if (intStatus)
    {
        s_frameDone = true;
    }
}




int main()
{
    uint8_t ch; //用于暂存串口收到的字符

    uint32_t frameBufferIndex = 0;

    system_clock_init();
    rgb_led_init();           //初始化 RGB 灯，初始化后 默认所有灯都不亮。
    interrupt_button2_init(); //初始化引脚，和引脚的中断方式以及开启引脚中断。
    uart_init();
    UART_WriteBlocking(UART1, txbuff, sizeof(txbuff) - 1);

    lcdif_pin_config();         //初始 lcdif 引脚
    lcdif_clock_init();         //初始化时钟
    BOARD_InitLcd();            //复位LCD
    SystemInstallIrqHandler(LCDIF_IRQn, (system_irq_handler_t)(uint32_t)APP_LCDIF_IRQHandler, NULL); // 设置中断服务函数

    ELCDIF_RgbModeInit();  // 初始化 elcdf 位 RGB 888 模式
    GIC_EnableIRQ(LCDIF_IRQn); //开启中断

    // UART_WriteByte(UART1,"")

    APP_FillFrameBuffer(s_frameBuffer[frameBufferIndex]);
    LCDIF->CTRL1_SET |= (0x2000);  // 使能 elcdf 一帧传输完成中断



    LCDIF->CTRL_SET |= 0x1;//开启 elcdf 开始显示
    LCDIF->CTRL_SET |= (1 << 17);


    while (1)
    {
        frameBufferIndex ^= 1U;
        APP_FillFrameBuffer(s_frameBuffer[frameBufferIndex]);

        LCDIF->NEXT_BUF = (uint32_t)s_frameBuffer[frameBufferIndex];
        
        s_frameDone = false;
        /* Wait for previous frame complete. */
        while (!s_frameDone)
        {
        }

    }

    return 0;
}
