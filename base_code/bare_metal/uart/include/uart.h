
#ifndef uart_h
#define uart_h


#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"

#define  uint32_t  unsigned int
#define  uint64_t  unsigned long int



/*定义 UART1 RX 引脚*/
#define UART1_RX_GPIO                GPIO1
#define UART1_RX_GPIO_PIN            (17U)
#define UART1_RX_IOMUXC              IOMUXC_UART1_RX_DATA_UART1_RX

/*定义 UART1 TX 引脚*/
#define UART1_TX_GPIO              GPIO1
#define UART1_TX_GPIO_PIN          (16U)
#define UART1_TX_IOMUXC            IOMUXC_UART1_TX_DATA_UART1_TX


/*******************************************************************************
 * uart引脚配置
 ******************************************************************************/
#define UART_RX_PAD_CONFIG_DATA            (SRE_0_SLOW_SLEW_RATE| \
                                        DSE_6_R0_6| \
                                        SPEED_1_MEDIUM_100MHz| \
                                        ODE_0_OPEN_DRAIN_DISABLED| \
                                        PKE_1_PULL_KEEPER_ENABLED| \
                                        PUE_1_PULL_SELECTED| \
                                        PUS_3_22K_OHM_PULL_UP| \
                                        HYS_0_HYSTERESIS_DISABLED) 
    /* 配置说明 : */
    /* 转换速率: 转换速率慢
        驱动强度: R0/6 
        带宽配置 : medium(100MHz)
        开漏配置: 关闭 
        拉/保持器配置: 使能
        拉/保持器选择: 上下拉
        上拉/下拉选择: 22K欧姆上拉(选择了保持器此配置无效)
        滞回器配置: 禁止 */ 

#define UART_TX_PAD_CONFIG_DATA            (SRE_0_SLOW_SLEW_RATE| \
                                        DSE_6_R0_6| \
                                        SPEED_1_MEDIUM_100MHz| \
                                        ODE_0_OPEN_DRAIN_DISABLED| \
                                        PKE_1_PULL_KEEPER_ENABLED| \
                                        PUE_0_KEEPER_SELECTED| \
                                        PUS_3_22K_OHM_PULL_UP| \
                                        HYS_0_HYSTERESIS_DISABLED)
    /* 配置说明 : */
    /* 转换速率: 转换速率慢
        驱动强度: R0/6 
        带宽配置 : medium(100MHz)
        开漏配置: 关闭 
        拉/保持器配置: 使能
        拉/保持器选择: 保持器
        上拉/下拉选择: 22K欧姆上拉(选择了保持器此配置无效)
        滞回器配置: 禁止 */ 



void uart_init(void);
int32_t UART_SetBaudRate(UART_Type *base, uint32_t baudRate_Bps, uint32_t srcClock_Hz);

void UART_WriteBlocking(UART_Type *base, const uint8_t *data, uint8_t length);
void UART_ReadBlocking(UART_Type *base, uint8_t *data, uint8_t length);

#endif

