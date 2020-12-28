#include "elcdif.h"
#include "MCIMX6Y2.h"




/*定义 elcdf 缓冲区*/
uint32_t s_frameBuffer[2][APP_IMG_HEIGHT][APP_IMG_WIDTH];




/* elcdif 显示接口外部引脚初始化
*
*/
void lcdif_pin_config(void)
{
    IOMUXC_SetPinMux(IOMUXC_LCD_CLK_LCDIF_CLK, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_CLK_LCDIF_CLK, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA00_LCDIF_DATA00, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA00_LCDIF_DATA00, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA01_LCDIF_DATA01, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA01_LCDIF_DATA01, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA02_LCDIF_DATA02, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA02_LCDIF_DATA02, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA03_LCDIF_DATA03, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA03_LCDIF_DATA03, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA04_LCDIF_DATA04, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA04_LCDIF_DATA04, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA05_LCDIF_DATA05, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA05_LCDIF_DATA05, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA06_LCDIF_DATA06, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA06_LCDIF_DATA06, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_LCDIF_DATA07, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_LCDIF_DATA07, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA08_LCDIF_DATA08, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA08_LCDIF_DATA08, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA09_LCDIF_DATA09, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA09_LCDIF_DATA09, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA10_LCDIF_DATA10, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA10_LCDIF_DATA10, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA11_LCDIF_DATA11, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA11_LCDIF_DATA11, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA12_LCDIF_DATA12, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA12_LCDIF_DATA12, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA13_LCDIF_DATA13, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA13_LCDIF_DATA13, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA14_LCDIF_DATA14, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA14_LCDIF_DATA14, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_LCDIF_DATA15, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_LCDIF_DATA15, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA16_LCDIF_DATA16, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA16_LCDIF_DATA16, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA17_LCDIF_DATA17, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA17_LCDIF_DATA17, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA18_LCDIF_DATA18, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA18_LCDIF_DATA18, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA19_LCDIF_DATA19, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA19_LCDIF_DATA19, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA20_LCDIF_DATA20, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA20_LCDIF_DATA20, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA21_LCDIF_DATA21, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA21_LCDIF_DATA21, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA22_LCDIF_DATA22, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA22_LCDIF_DATA22, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_LCDIF_DATA23, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_LCDIF_DATA23, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_ENABLE_LCDIF_ENABLE, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_ENABLE_LCDIF_ENABLE, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_HSYNC_LCDIF_HSYNC, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_HSYNC_LCDIF_HSYNC, LCD_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_LCDIF_VSYNC, 0U);
    IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_LCDIF_VSYNC, LCD_PAD_CONFIG_DATA);
}





/*初始化 elcdf 的时钟
*/
void lcdif_clock_init(void)
{
    /*设置 PLL5  的输出时钟*/
    CCM_ANALOG->PLL_VIDEO_NUM &= (0x3 << 30);   //清零PLL 分数分频的分子寄存器
    CCM_ANALOG->PLL_VIDEO_DENOM &= (0x3 << 30); //清零PLL 分数分频的分母寄存器

    /*
     * 设置时钟分频
     *
     * ------------------------------------------------------------------------
     * |        分频数       | PLL_VIDEO[POST_DIV_SELECT]  | MISC2[VIDEO_DIV] |
     * ------------------------------------------------------------------------
     * |         1           |            2                |        0         |
     * ------------------------------------------------------------------------
     * |         2           |            1                |        0         |
     * ------------------------------------------------------------------------
     * |         4           |            2                |        3         |
     * ------------------------------------------------------------------------
     * |         8           |            1                |        3         |
     * ------------------------------------------------------------------------
     * |         16          |            0                |        3         |
     * ------------------------------------------------------------------------
     */
    CCM_ANALOG->PLL_VIDEO = 0;
    CCM_ANALOG->PLL_VIDEO &= ~(0x3 << 19); // 清零PLL_VIDEO[POST_DIV_SELECT]
    CCM_ANALOG->PLL_VIDEO |= (0x01 << 19); //设置分频系数为2

    CCM_ANALOG->MISC2 &= ~(0xC0000000); //清零VIDEO_DIV位
    CCM_ANALOG->MISC2 |= (0x3 << 30);// 配合CCM_ANALOG->PLL_VIDEO寄存器设置时钟分频


    CCM_ANALOG->PLL_VIDEO &= ~(0x7F); // 清零时钟分频
    CCM_ANALOG->PLL_VIDEO |= (0x1F);  //设置时钟分频为 31(十进制)

    CCM_ANALOG->PLL_VIDEO |= 1 << 13; //使能PLL5时钟输出

    /*等待设置生效*/
    while ((CCM_ANALOG->PLL_VIDEO & CCM_ANALOG_PLL_VIDEO_LOCK_MASK) == 0)
    {
    }

    /*设置从PLL5  到 elcdf 根时钟所经过的时钟选择和时钟分频寄存器*/
    CCM->CSCDR2 &= ~(0x07 << 15); //清零
    CCM->CSCDR2 |= (0x02 << 15);  //设置CSCDR2[LCDIF1_PRE_CLK_SEL] 选择 PLL5 输出时钟

    CCM->CSCDR2 &= ~(0x07 << 12); //清零
    CCM->CSCDR2 |= (0x01 << 12);  //设置 CSCDR2[LCDIF1_PRED]时钟分频值

    CCM->CBCMR &= ~(0x07 << 23); //清零CBCMR[LCDIF1_PODF] 时钟分频值
    CCM->CBCMR |= (0x01 << 23);

    CCM->CSCDR2 &= ~(0x07 << 9); //清零
    CCM->CSCDR2 |= (0x00 << 9);  //选择 CSCDR2[LCDIF1_CLK_SEL] 选择 PLL5 输出时钟
}





/* 初始化 LCD_DISP,点亮 显示屏 */
void BOARD_InitLcd(void)
{
    volatile uint32_t i = 0x100U; //大致设定elcdf复位信号的持续时间

    GPIO5->GDIR |= (1 << 9); //设置GPIO5_09为输出模式
    GPIO5->DR &= ~(1 << 9);  //设置GPIO5_09输出电平为低电平

    /*等待一段时间，elcdf复位信号保持一段时间*/
    while (i--)
    {
    }
    GPIO5->DR |= (1 << 9); //设置GPIO5_09输出电平为高电平。 完成elcdf复位

    /* 设置 LCD D的背光控制引脚为高电平 */
    CCM_CCGR1_CG13(0x3);     //开启GPIO1的时钟
    GPIO1->GDIR |= (1 << 8); //设置GPIO1_08为输出模式
    GPIO1->DR |= (1 << 8);   //设置GPIO1_08输出电平为高电平
}




/*复位 elcdf*/
void ELCDIF_Reset(void)
{
    volatile uint32_t i = 0x100;

    /* Disable the clock gate. */
    LCDIF->CTRL_CLR = LCDIF_CTRL_CLKGATE_MASK;
    /* Confirm the clock gate is disabled. */
    while (LCDIF->CTRL & LCDIF_CTRL_CLKGATE_MASK)
    {
    }

    /* Reset the block. */
    LCDIF->CTRL_SET = LCDIF_CTRL_SFTRST_MASK;
    /* Confirm the reset bit is set. */
    while (!(LCDIF->CTRL & LCDIF_CTRL_SFTRST_MASK))
    {
    }

    /* Delay for the reset. */
    while (i--)
    {
    }

    /* Bring the module out of reset. */
    LCDIF->CTRL_CLR = LCDIF_CTRL_SFTRST_MASK;
    /* Disable the clock gate. */
    LCDIF->CTRL_CLR = LCDIF_CTRL_CLKGATE_MASK;
}



/*将 elcdf 初始化为 rgb 888 模式
*/
void ELCDIF_RgbModeInit(void)
{

    CCM_CCGR3_CG5(0x3); //开启GPIO5的时钟
    /* Reset. */
    ELCDIF_Reset();


    LCDIF->CTRL &= ~(0x300); //根据颜色格式设置 CTRL 寄存器 颜色个事为RGB888
    LCDIF->CTRL |= (0x3 << 8);

    LCDIF->CTRL &= ~(0xC00); //设置数据宽度为24位宽
    LCDIF->CTRL |= (0x3 << 10);

    LCDIF->CTRL |= (0x20000); // 选择 RGB 模式
    LCDIF->CTRL |= (0x80000); // 选择 RGB 模式 开启显示
    LCDIF->CTRL |= (0x20);    //设置elcdf接口为主模式



    LCDIF->CTRL1 &= ~(0xF0000);   //清零32位数据有效位
    LCDIF->CTRL1 |= (0x07 << 16); // 设置32位有效位的低24位有效。


    // LCDIF->TRANSFER_COUNT = 0;//清零分辨率设置寄存器
    LCDIF->TRANSFER_COUNT |= APP_IMG_HEIGHT << 16; //设置一列 像素数  480
    LCDIF->TRANSFER_COUNT |= APP_IMG_WIDTH << 0;   //设置一行 像素数  800



    LCDIF->VDCTRL0 |= LCDIF_VDCTRL0_ENABLE_PRESENT_MASK;         //生成使能信号
    LCDIF->VDCTRL0 |= LCDIF_VDCTRL0_VSYNC_PERIOD_UNIT_MASK;      //设置VSYNC周期 的单位为显示时钟的时钟周期
    LCDIF->VDCTRL0 |= LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_UNIT_MASK; //设置VSYNC 脉冲宽度的单位为显示时钟的时钟周期


    LCDIF->VDCTRL0 |= (1 << 24);    //设置 数据使能信号的有效电平为高电平
    LCDIF->VDCTRL0 &= ~(0x8000000); //设置 VSYNC 有效电平为低电平
    LCDIF->VDCTRL0 &= ~(0x4000000); //设置HSYNC有效电平为低电平
    LCDIF->VDCTRL0 |= (0x2000000);  // 设置在时钟的下降沿输出数据，在时钟的上升沿捕获数据。

    LCDIF->VDCTRL0 |= APP_VSW;


    //     以显示时钟为单位的周期。

    LCDIF->VDCTRL1 = APP_VSW + APP_IMG_HEIGHT + APP_VFP + APP_VBP; //设置VSYNC 信号周期

    LCDIF->VDCTRL2 |= (APP_HSW << 18);                               //HSYNC 信号有效电平长度
    LCDIF->VDCTRL2 |= (APP_HFP + APP_HBP + APP_IMG_WIDTH + APP_HSW); //HSYNC 信号周期

    LCDIF->VDCTRL3 |= (APP_HBP + APP_HSW) << 16;
    LCDIF->VDCTRL3 |= (APP_VBP + APP_VSW);

    LCDIF->VDCTRL4 |= (0x40000);
    LCDIF->VDCTRL4 |= (APP_IMG_WIDTH << 0);

    LCDIF->CUR_BUF = (uint32_t)s_frameBuffer[0];
    LCDIF->NEXT_BUF = (uint32_t)s_frameBuffer[0];
}

// void APP_ELCDIF_Init(void)
// {
//     lcdif_pin_config();
//     ELCDIF_RgbModeInit(APP_ELCDIF);
// }




void APP_FillFrameBuffer(uint32_t frameBuffer[APP_IMG_HEIGHT][APP_IMG_WIDTH])
{
    /* Background color. */
    static const uint32_t bgColor = 0U;
    /* Foreground color. */
    static uint8_t fgColorIndex = 0U;
    static const uint32_t fgColorTable[] = {0x000000FFU, 0x0000FF00U, 0x0000FFFFU, 0x00FF0000U,
                                            0x00FF00FFU, 0x00FFFF00U, 0x00FFFFFFU};
    uint32_t fgColor = fgColorTable[fgColorIndex];

    /* Position of the foreground rectangle. */
    static uint16_t upperLeftX = 0U;
    static uint16_t upperLeftY = 0U;
    static uint16_t lowerRightX = (APP_IMG_WIDTH - 1U) / 2U;
    static uint16_t lowerRightY = (APP_IMG_HEIGHT - 1U) / 2U;

    static int8_t incX = 1;
    static int8_t incY = 1;

    /* Change color in next forame or not. */
    static unsigned char changeColor = false;

    uint32_t i, j;

    /* Background color. */
    for (i = 0; i < APP_IMG_HEIGHT; i++)
    {
        for (j = 0; j < APP_IMG_WIDTH; j++)
        {
            frameBuffer[i][j] = bgColor;
        }
    }

    /* Foreground color. */
    for (i = upperLeftY; i < lowerRightY; i++)
    {
        for (j = upperLeftX; j < lowerRightX; j++)
        {
            frameBuffer[i][j] = fgColor;
        }
    }

    /* Update the format: color and rectangle position. */
    upperLeftX += incX;
    upperLeftY += incY;
    lowerRightX += incX;
    lowerRightY += incY;

    changeColor = false;

    if (0U == upperLeftX)
    {
        incX = 1;
        changeColor = true;
    }
    else if (APP_IMG_WIDTH - 1 == lowerRightX)
    {
        incX = -1;
        changeColor = true;
    }

    if (0U == upperLeftY)
    {
        incY = 1;
        changeColor = true;
    }
    else if (APP_IMG_HEIGHT - 1 == lowerRightY)
    {
        incY = -1;
        changeColor = true;
    }

    if (changeColor)
    {
        fgColorIndex++;

        if (ARRAY_SIZE(fgColorTable) == fgColorIndex)
        {
            fgColorIndex = 0U;
        }
    }
}

