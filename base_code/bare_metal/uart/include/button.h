#ifndef button_h
#define button_h


#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"
#include "system_MCIMX6Y2.h"


/*按键2 GPIO端口、引脚号及IOMUXC复用宏定义*/
#define button2_GPIO               GPIO5
#define button2_GPIO_PIN           (1U)
#define button2_IOMUXC             IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01




/* 按键PAD配置 */
#define button_PAD_CONFIG_DATA            (SRE_0_SLOW_SLEW_RATE| \
                                        DSE_6_R0_6| \
                                        SPEED_2_MEDIUM_100MHz| \
                                        ODE_0_OPEN_DRAIN_DISABLED| \
                                        PKE_0_PULL_KEEPER_DISABLED| \
                                        PUE_0_KEEPER_SELECTED| \
                                        PUS_0_100K_OHM_PULL_DOWN| \
                                        HYS_1_HYSTERESIS_ENABLED)   
    /* 配置说明 : */
    /* 转换速率: 转换速率慢
      驱动强度: R0/6 
      带宽配置 : medium(100MHz)
      开漏配置: 关闭 
      拉/保持器配置: 关闭
      拉/保持器选择: 保持器（上面已关闭，配置无效）
      上拉/下拉选择: 100K欧姆下拉（上面已关闭，配置无效）
      滞回器配置: 开启 */ 



/*函数*/
void button2_init(void);
int get_button2_status(void);
void interrupt_button2_init(void);
void EXAMPLE_GPIO_IRQHandler(void);


#endif