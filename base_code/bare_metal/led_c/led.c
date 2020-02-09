
#define CCM_CCGR1 (volatile unsigned long*)0x20C406C          //时钟控制寄存器
#define IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04 (volatile unsigned long*)0x20E006C//GPIO1_04复用功能选择寄存器
#define IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04 (volatile unsigned long*)0x20E02F8 //PAD属性设置寄存器
#define GPIO1_GDIR (volatile unsigned long*)0x0209C004 //GPIO方向设置寄存器（输入或输出）
#define GPIO1_DR (volatile unsigned long*)0x0209C000   //GPIO输出状态寄存器


#define uint32_t  unsigned int 

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
    *(CCM_CCGR1) = 0xFFFFFFFF;   //开启GPIO1的时钟
    *(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04) = 0x5;//设置PAD复用功能为GPIO
    *(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04) = 0x1F838;//设置PAD属性
    *(GPIO1_GDIR) = 0x10;//设置GPIO为输出模式
    *(GPIO1_DR) = 0x0;   //设置输出电平为低电平

    while(1)
    {
        *(GPIO1_DR) = 0x0;
        delay(0xFFFFF);
        *(GPIO1_DR) = 1<<4;
        delay(0xFFFFF);


    }

    return 0;    
}




