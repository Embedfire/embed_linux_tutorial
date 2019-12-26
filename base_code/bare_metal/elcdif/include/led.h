#ifndef led_h
#define led_h

#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"

/*LED GPIO端口、引脚号及IOMUXC复用宏定义*/
#define RGB_RED_LED_GPIO                GPIO1
#define RGB_RED_LED_GPIO_PIN            (4U)
#define RGB_RED_LED_IOMUXC              IOMUXC_GPIO1_IO04_GPIO1_IO04

#define RGB_GREEN_LED_GPIO              GPIO4
#define RGB_GREEN_LED_GPIO_PIN          (20U)
#define RGB_GREEN_LED_IOMUXC            IOMUXC_CSI_HSYNC_GPIO4_IO20

#define RGB_BLUE_LED_GPIO               GPIO4
#define RGB_BLUE_LED_GPIO_PIN           (19U)
#define RGB_BLUE_LED_IOMUXC             IOMUXC_CSI_VSYNC_GPIO4_IO19



/* 所有引脚均使用同样的PAD配置 */
#define LED_PAD_CONFIG_DATA            (SRE_0_SLOW_SLEW_RATE| \
                                        DSE_6_R0_6| \
                                        SPEED_2_MEDIUM_100MHz| \
                                        ODE_0_OPEN_DRAIN_DISABLED| \
                                        PKE_0_PULL_KEEPER_DISABLED| \
                                        PUE_0_KEEPER_SELECTED| \
                                        PUS_0_100K_OHM_PULL_DOWN| \
                                        HYS_0_HYSTERESIS_DISABLED)   
    /* 配置说明 : */
    /* 转换速率: 转换速率慢
      驱动强度: R0/6 
      带宽配置 : medium(100MHz)
      开漏配置: 关闭 
      拉/保持器配置: 关闭
      拉/保持器选择: 保持器（上面已关闭，配置无效）
      上拉/下拉选择: 100K欧姆下拉（上面已关闭，配置无效）
      滞回器配置: 关闭 */  


/*RGB led 开关控制宏定义*/
#define red_led_on   GPIO1->DR &= ~(1<<4)
#define red_led_off  GPIO1->DR |= (1<<4)

#define green_led_on   GPIO4->DR &= ~(1<<20)
#define green_led_off  GPIO4->DR |= (1<<20)

#define blue_led_on    GPIO4->DR &= ~(1<<19)
#define blue_led_off  GPIO4->DR |= (1<<19)


/*函数*/
void rgb_led_init(void);


#endif