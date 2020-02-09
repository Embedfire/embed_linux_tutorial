/*说明：
 * 按键对应Pro 开发板 button2 ，button2输入引脚接有下拉电阻，默认低电平，按键按下后变为高电平。
 * 按键初始化函数有两个，函数button2_init 仅将引脚设置为输入，通过轮询检测按键状态。函数interrupt_button2_init
 * 初始化了GPIO中断，上升沿触发.
*/
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


extern uint8_t button_status;  //按键状态标记 0，按键未按下。1，按键按下
/*按键初始化函数*/
void interrupt_button2_init(void)
{
    volatile uint32_t *icr;  //用于保存 GPIO-ICR寄存器的地址,与 icrShift 变量配合使用
    uint32_t icrShift;       //引脚号大于16时会用到,

    icrShift = button2_GPIO_PIN;  //保存button2引脚对应的 GPIO 号

    /*添加中断服务函数到  "中断向量表"*/
    SystemInstallIrqHandler(GPIO5_Combined_0_15_IRQn, (system_irq_handler_t)EXAMPLE_GPIO_IRQHandler, NULL);
    GIC_EnableIRQ(GPIO5_Combined_0_15_IRQn);                 //开启中断


    CCM_CCGR1_CG15(0x3);  //开启GPIO5的时钟

    /*设置 按键引脚的PAD属性*/
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01,0);     
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, button_PAD_CONFIG_DATA); 
    
    /*设置GPIO方向（输入或输出）*/
    GPIO5->IMR &= ~(1 << button2_GPIO_PIN);  //寄存器重置为默认值
    GPIO5->GDIR &= ~(1<<1);                  //设置GPIO5_01为输入模式

    /*设置GPIO引脚中断类型*/
    GPIO5->EDGE_SEL &= ~(1U << button2_GPIO_PIN);//寄存器重置为默认值

    if(button2_GPIO_PIN < 16)
    {
        icr = &(GPIO5->ICR1);
    }
    else
    {
        icr = &(GPIO5->ICR2);
        icrShift -= 16;
    }

    /*按键引脚默认低电平，设置为上升沿触发中断*/
     *icr = (*icr & (~(3U << (2 * icrShift)))) | (2U << (2 * icrShift));

     button2_GPIO->IMR |= (1 << button2_GPIO_PIN); //使能GPIO引脚中断
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


/*按键中断处理函数*/
void EXAMPLE_GPIO_IRQHandler(void)
{
    /*按键引脚中断服务函数*/
    button2_GPIO->ISR = 1U << button2_GPIO_PIN;  //清除GIIP中断标志位
    if(button_status > 0)
    {
        button_status = 0;
    }
    else
    {
        button_status = 1;
    }
}



