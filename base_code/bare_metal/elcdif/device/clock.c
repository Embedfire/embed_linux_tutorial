#include "clock.h"
#include "MCIMX6Y2.h"
#include "system_MCIMX6Y2.h"

/* 外部 XTAL (OSC) 时钟频率 */
uint32_t g_xtalFreq = 24000000;
/*外部 RTC XTAL 时钟频率 */
uint32_t g_rtcXtalFreq = 32768;

void system_clock_init(void)
{
    /******************* PLL 输出时钟设置************************/
    if ((CCM->CCSR & (0x01 << 2)) == 0) //CPU 使用的是 ARM PLL
    {
        /*将CPU时钟切换到XTAL (OSC) 时钟*/                   
        CCM->CCSR &= ~(0x01 << 8); //控制CCSR: step_sel ，选择 osc_clk 作为时钟源
        CCM->CCSR |= (0x01 << 2);  //设置GLITCHLESS MUX 选择 step_clk 作为时钟源
    }

    /*设置PLL1输出时钟为792MHz，它将作为CPU时钟*/
    CCM_ANALOG->PLL_ARM |= (0x42 << 0);



    /*将CPU 时钟重新切换到 ARM PLL*/
    CCM->CCSR &= ~(0x01 << 2);

    /*设置时钟分频系数为0，即不分频*/
    // CCM->CACRR &= ~(0x07 << 0); //清零分频寄存器   不分频
    CCM->CACRR |= (0x07 << 0);     // 8分频


    /*设置PLL2(System PLL) 输出时钟*/
    /* Configure SYS PLL to 528M */
    CCM_ANALOG->PLL_SYS_SS &= ~(0x8000);     //使能PLL2 PFD输出
    CCM_ANALOG->PLL_SYS_NUM &= ~(0x3FFFFFFF);//设置分频系数为0，即不分频。
    CCM_ANALOG->PLL_SYS |= (0x2000); //使能PLL2 输出
    CCM_ANALOG->PLL_SYS |= (1 << 0); //设置输出频率为528M
    while ((CCM_ANALOG->PLL_SYS & (0x80000000)) == 0) //等待设置生效
    {
    }

    /*设置PLL3(System PLL) 输出时钟*/
    /* Configure USB PLL to 480M */
    CCM_ANALOG->PLL_USB1 |= (0x2000);    //使能 PLL3时钟输出
    CCM_ANALOG->PLL_USB1 |= (0x1000);    //PLL3上电使能
    CCM_ANALOG->PLL_USB1 |= (0x40);      // 使能USBPHYn
    CCM_ANALOG->PLL_USB1 &= ~(0x01 << 0);//设置输出频率为480MHz
    while ((CCM_ANALOG->PLL_SYS & (0x80000000)) == 0)//等待设置生效
    {
    }

    /*关闭暂时不使用的 PLL4 、PLL5  、PLL6 、PLL7*/
    CCM_ANALOG->PLL_AUDIO = (0x1000);    //关闭PLL4
    CCM_ANALOG->PLL_VIDEO = (0x1000);    //关闭PLL5
    CCM_ANALOG->PLL_ENET =  (0x1000);    //关闭PLL6
    CCM_ANALOG->PLL_USB2 =  (0x00);           //关闭PLL7

 
    /******************PFD 输出时钟设置*******************/
    /*禁用PLL2 的所有PFD输出*/
    CCM_ANALOG->PFD_528 |=(0x80U) ;      //关闭PLL2 PFD0
    CCM_ANALOG->PFD_528 |=(0x8000U) ;    //关闭PLL2 PFD1
    // CCM_ANALOG->PFD_528 |=(0x800000U) ;  //关闭PLL2 PFD2 ,DDR使用的是该时钟源，关闭后程序不能运行。暂时不关闭
    CCM_ANALOG->PFD_528 |=(0x80000000U); //关闭PLL2 PFD3
    
    /*设置PLL2 的PFD输出频率*/
    CCM_ANALOG->PFD_528 &= ~(0x3FU); //清零PLL2 PFD0 时钟分频
    CCM_ANALOG->PFD_528 &= ~(0x3F00U); //清零PLL2 PFD1 时钟分频
    CCM_ANALOG->PFD_528 &= ~(0x3F00U); //清零PLL2 PFD2 时钟分频
    CCM_ANALOG->PFD_528 &= ~(0x3F00U); //清零PLL2 PFD3 时钟分频

    CCM_ANALOG->PFD_528 |= (0x1B << 0); //设置PLL2 PFD0 输出频率为 352M
    CCM_ANALOG->PFD_528 |= (0x10 << 8); //设置PLL2 PFD0 输出频率为 594M
    CCM_ANALOG->PFD_528 |= (0x18 << 16); //设置PLL2 PFD0 输出频率为 396M
    CCM_ANALOG->PFD_528 |= (0x30 << 24); //设置PLL2 PFD0 输出频率为 198M

    /*启用PLL2 的所有PFD输出*/
    CCM_ANALOG->PFD_528 &= ~(0x80U) ;      //开启PLL2 PFD0
    CCM_ANALOG->PFD_528 &= ~(0x8000U) ;    //开启PLL2 PFD1
    CCM_ANALOG->PFD_528 &= ~(0x800000U) ;  //开启PLL2 PFD2
    CCM_ANALOG->PFD_528 &= ~(0x80000000U); //开启PLL2 PFD3


    /*禁用PLL3 的所有PFD输出*/
    CCM_ANALOG->PFD_480 |=(0x80U) ;      //关闭PLL3 PFD0
    CCM_ANALOG->PFD_480 |=(0x8000U) ;    //关闭PLL3 PFD1
    CCM_ANALOG->PFD_480 |=(0x800000U) ;  //关闭PLL3 PFD2
    CCM_ANALOG->PFD_480 |=(0x80000000U); //关闭PLL3 PFD3

    /*设置PLL3 的PFD输出频率*/
    CCM_ANALOG->PFD_480 &= ~(0x3FU);   //清零PLL3 PFD0 时钟分频
    CCM_ANALOG->PFD_480 &= ~(0x3F00U); //清零PLL3 PFD1 时钟分频
    CCM_ANALOG->PFD_480 &= ~(0x3F00U); //清零PLL3 PFD2 时钟分频
    CCM_ANALOG->PFD_480 &= ~(0x3F00U); //清零PLL3 PFD3 时钟分频

    CCM_ANALOG->PFD_480 |= (0xC << 0); //设置PLL3 PFD0 输出频率为 720M
    CCM_ANALOG->PFD_480 |= (0x10 << 8); //设置PLL3 PFD0 输出频率为 540M
    CCM_ANALOG->PFD_480 |= (0x11 << 16); //设置PLL3 PFD0 输出频率为 508.2M
    CCM_ANALOG->PFD_480 |= (0x13 << 24); //设置PLL3 PFD0 输出频率为 454.7M

    /*启用PLL3 的所有PFD输出*/
    CCM_ANALOG->PFD_480 &= ~(0x80U) ;      //开启PLL3 PFD0
    CCM_ANALOG->PFD_480 &= ~(0x8000U) ;    //开启PLL3 PFD1
    CCM_ANALOG->PFD_480 &= ~(0x800000U) ;  //开启PLL3 PFD2
    CCM_ANALOG->PFD_480 &= ~(0x80000000U); //开启PLL3 PFD3
  

    /******************常用外设根时钟设置****************/
    CCM->CSCDR1 &= ~(0x01 << 6); //设置UART选择 PLL3 / 6 = 80MHz
    CCM->CSCDR1 &= ~(0x3F);     //清零
    CCM->CSCDR1 |= (0x01 << 0); //设置串口根时钟分频值为1，UART根时钟频率为：80M / (dev + 1) = 40MHz
}

