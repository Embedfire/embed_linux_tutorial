#include "uart.h"

void uart_init(void)
{
    /*时钟初始化，设置 UART 根时钟，并设置为40MHz*/
    CCM->CSCDR1 &= ~(0x01 << 6); //设置UART选择 PLL3 / 6 = 80MHz
    CCM->CSCDR1 &= ~(0x3F);      //清零
    CCM->CSCDR1 |= (0x01 << 0);  //设置串口根时钟分频值为1，UART根时钟频率为：80M / (dev + 1) = 40MHz

    /*开启 UART1 的时钟*/
    CCM_CCGR5_CG12(0x3); //开启UART1的时钟

    UART1->UCR1 &= ~UART_UCR1_UARTEN_MASK; //禁用 UART1

    /*软件复位*/
    UART1->UCR2 &= ~UART_UCR2_SRST_MASK;
    while ((UART1->UCR2 & UART_UCR2_SRST_MASK) == 0)
    {
    }

    UART1->UCR1 = 0x0;
    UART1->UCR2 = UART_UCR2_SRST_MASK;
    UART1->UCR3 = UART_UCR3_DSR_MASK | UART_UCR3_DCD_MASK | UART_UCR3_RI_MASK;
    UART1->UCR4 = UART_UCR4_CTSTL(32);
    UART1->UFCR = UART_UFCR_TXTL(2) | UART_UFCR_RXTL(1);
    UART1->UESC = UART_UESC_ESC_CHAR(0x2B);
    UART1->UTIM = 0x0;
    UART1->ONEMS = 0x0;
    UART1->UTS = UART_UTS_TXEMPTY_MASK | UART_UTS_RXEMPTY_MASK;
    UART1->UMCR = 0x0;

    /*引脚初始化*/
    IOMUXC_SetPinMux(UART1_RX_IOMUXC, 0);
    IOMUXC_SetPinConfig(UART1_RX_IOMUXC, UART_RX_PAD_CONFIG_DATA);

    IOMUXC_SetPinMux(UART1_TX_IOMUXC, 0);
    IOMUXC_SetPinConfig(UART1_TX_IOMUXC, UART_TX_PAD_CONFIG_DATA);

    /*******uart初始化******/
    /*设置控制寄存器到默认值*/
    UART1->UCR2 |= (1 << 5);  //8位数宽度
    UART1->UCR2 &= ~(1 << 6); //一位停止位
    UART1->UCR2 &= ~(1 << 8); //禁用奇偶校验位

    UART1->UCR2 |= (1 << 2);  //使能发送
    UART1->UCR2 |= (1 << 1);  //使能接收
    UART1->UCR2 |= (1 << 14); //忽略流控

    /* For imx family device, UARTs are used in  mode, so that this bit should always be set.*/
    UART1->UCR3 |= UART_UCR3_RXDMUXSEL_MASK;

    UART1->UFCR = (UART1->UFCR & ~UART_UFCR_TXTL_MASK) | UART_UFCR_TXTL(1); //设置发送FIFO 阀值
    UART1->UFCR = (UART1->UFCR & ~UART_UFCR_TXTL_MASK) | UART_UFCR_TXTL(1); //设置接收FIFO 阀值

    UART1->UCR1 &= ~UART_UCR1_ADBR_MASK; //禁用可变波特率
    // UART1->UCR1 |= UART_UCR1_ADBR_MASK;

    /*波特率设置方式 1 。 使用官方SDK设置波特率函数*/
    UART_SetBaudRate(UART1, 115200, 40000000);

    /*波特率设置方式 2 。 手动计算，填入寄存器*/
    /*设置串口波特率
    * Ref Freq时钟 40MHz
    * UFCR RFDIV   110  0x06 7分频    5.714MHz
    * BaudRate     115200bps
    * UBMR         31-1 = 0x09
    * UBIR         10-1 = 0x1E
    */
    UART1->UFCR &= ~(0x07 << 7); //清零分频值
    UART1->UFCR |= (0x06 << 7);  //设置分频值，40MHz /7 =  5.714MHz

    UART1->UBIR = 0x09;
    UART1->UBMR = 0x1E;

    /*开启串口*/
    UART1->UCR1 |= UART_UCR1_UARTEN_MASK;
}


/*!
 * 功能：官方SDK 串口字符串读取函数
 * @brief Reads the receiver register.
 *
 * This function is used to read data from receiver register.
 * The upper layer must ensure that the receiver register is full or that
 * the RX FIFO has data before calling this function.
 *
 * @param base UART peripheral base address.
 * @return Data read from data register.
 */
static inline uint8_t UART_ReadByte(UART_Type *base)
{
    return (uint8_t)((base->URXD & UART_URXD_RX_DATA_MASK) >> UART_URXD_RX_DATA_SHIFT);
}


/*函数功能：串口接收函数
 *参数： base,指定串口。data,保存接收到的数据。 length，要接收的数据长度
 *
*/
void UART_ReadBlocking(UART_Type *base, uint8_t *data, uint8_t length)
{
    while (length--)
    {
        /* 等待接收完成 */
        while (!(base->USR2 & UART_USR2_RDR_MASK))
        {
        }
        /*读取接收到的数据 */
        *(data++) = UART_ReadByte(base);
    }
}


/*!
 * 功能：官方SDK 串口发送函数
 * 参数：base，指定串口。data，指定要发送的字节
 * This function is used to write data to transmitter register.
 * The upper layer must ensure that the TX register is empty or that
 * the TX FIFO has room before calling this function.
 */
static inline void UART_WriteByte(UART_Type *base, uint8_t data)
{
    base->UTXD = data & UART_UTXD_TX_DATA_MASK;
}

/*
 *功能：官方SDK 串口字符串发送函数
 *参数说明：
*/
void UART_WriteBlocking(UART_Type *base, const uint8_t *data, uint8_t length)
{

    while (length--)
    {
        /* Wait for TX fifo valid.
         * This API can only ensure that the data is written into the data buffer but can't
         * ensure all data in the data buffer are sent into the transmit shift buffer.
         */
        while (!(base->USR2 & UART_USR2_TXDC_MASK))
        {
        }
        UART_WriteByte(base, *(data++));
    }
}

/* 官方SDK 波特率设置函数，
 * 修改内容：修改了函数的返回值，波特率设置成功，返回1 。波特率设置失败返回 0
 *This UART instantiation uses a slightly different baud rate calculation.
 * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
 * To get a baud rate, three register need to be writen, UFCR,UBMR and UBIR
 * At first, find the approximately maximum divisor of src_Clock and baudRate_Bps.
 * If the numerator and denominator are larger then register maximum value(0xFFFF),
 * both of numerator and denominator will be divided by the same value, which
 * will ensure numerator and denominator range from 0~maximum value(0xFFFF).
 * Then calculate UFCR and UBIR value from numerator, and get UBMR value from denominator.
 */
int32_t UART_SetBaudRate(UART_Type *base, uint32_t baudRate_Bps, uint32_t srcClock_Hz)
{
    uint32_t numerator = 0u;
    uint32_t denominator = 0U;
    uint32_t divisor = 0U;
    uint32_t refFreqDiv = 0U;
    uint32_t divider = 1U;
    uint64_t baudDiff = 0U;
    uint64_t tempNumerator = 0U;
    uint32_t tempDenominator = 0u;

    /* get the approximately maximum divisor */
    numerator = srcClock_Hz;
    denominator = baudRate_Bps << 4;
    divisor = 1;

    while (denominator != 0)
    {
        divisor = denominator;
        denominator = numerator % denominator;
        numerator = divisor;
    }

    numerator = srcClock_Hz / divisor;
    denominator = (baudRate_Bps << 4) / divisor;

    /* numerator ranges from 1 ~ 7 * 64k */
    /* denominator ranges from 1 ~ 64k */
    if ((numerator > (UART_UBIR_INC_MASK * 7)) || (denominator > UART_UBIR_INC_MASK))
    {
        uint32_t m = (numerator - 1) / (UART_UBIR_INC_MASK * 7) + 1;
        uint32_t n = (denominator - 1) / UART_UBIR_INC_MASK + 1;
        uint32_t max = m > n ? m : n;
        numerator /= max;
        denominator /= max;
        if (0 == numerator)
        {
            numerator = 1;
        }
        if (0 == denominator)
        {
            denominator = 1;
        }
    }
    divider = (numerator - 1) / UART_UBIR_INC_MASK + 1;

    switch (divider)
    {
        case 1:
            refFreqDiv = 0x05;
            break;
        case 2:
            refFreqDiv = 0x04;
            break;
        case 3:
            refFreqDiv = 0x03;
            break;
        case 4:
            refFreqDiv = 0x02;
            break;
        case 5:
            refFreqDiv = 0x01;
            break;
        case 6:
            refFreqDiv = 0x00;
            break;
        case 7:
            refFreqDiv = 0x06;
            break;
        default:
            refFreqDiv = 0x05;
            break;
    }
    /* Compare the difference between baudRate_Bps and calculated baud rate.
     * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
     * baudDiff = (srcClock_Hz/divider)/( 16 * ((numerator / divider)/ denominator).
     */
    tempNumerator = srcClock_Hz;
    tempDenominator = (numerator << 4);
    divisor = 1;
    /* get the approximately maximum divisor */
    while (tempDenominator != 0)
    {
        divisor = tempDenominator;
        tempDenominator = tempNumerator % tempDenominator;
        tempNumerator = divisor;
    }
    tempNumerator = srcClock_Hz / divisor;
    tempDenominator = (numerator << 4) / divisor;
    baudDiff = (tempNumerator * denominator) / tempDenominator;
    baudDiff = (baudDiff >= baudRate_Bps) ? (baudDiff - baudRate_Bps) : (baudRate_Bps - baudDiff);

    if (baudDiff < (baudRate_Bps / 100) * 3)
    {
        base->UFCR &= ~UART_UFCR_RFDIV_MASK;
        base->UFCR |= UART_UFCR_RFDIV(refFreqDiv);
        base->UBIR = UART_UBIR_INC(denominator - 1);
        base->UBMR = UART_UBMR_MOD(numerator / divider - 1);
        base->ONEMS = UART_ONEMS_ONEMS(srcClock_Hz / (1000 * divider));

        return 1;
    }
    else
    {
        return 0;
    }
}