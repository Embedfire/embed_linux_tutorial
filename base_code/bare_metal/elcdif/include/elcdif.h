#ifndef elcdif_h
#define elcdif_h

#include "MCIMX6Y2.h"
#include "fsl_iomuxc.h"
#include "pad_config.h"


/*定义 显示屏信息 */
#define APP_IMG_HEIGHT 480    
#define APP_IMG_WIDTH 800
#define APP_HSW 41
#define APP_HFP 4
#define APP_HBP 8
#define APP_VSW 10
#define APP_VFP 4
#define APP_VBP 2

/* 定义 elcdf 显示控制引脚*/
#define LCD_DISP_GPIO GPIO5
#define LCD_DISP_GPIO_PIN 9

/* 定义 elcdf 背光控制引脚 */
#define LCD_BL_GPIO GPIO1
#define LCD_BL_GPIO_PIN 8

#define false 0
#define true 1
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* 所有引脚均使用同样的PAD配置 */
#define LCD_PAD_CONFIG_DATA (SRE_1_FAST_SLEW_RATE |      \
                             DSE_6_R0_6 |                \
                             SPEED_3_MAX_200MHz |        \
                             ODE_0_OPEN_DRAIN_DISABLED | \
                             PKE_1_PULL_KEEPER_ENABLED | \
                             PUE_0_KEEPER_SELECTED |     \
                             PUS_0_100K_OHM_PULL_DOWN |  \
                             HYS_0_HYSTERESIS_DISABLED)
/* 配置说明 : */
/* 转换速率: 转换速率快
        驱动强度: R0/6 
        带宽配置 : max(200MHz)
        开漏配置: 关闭 
        拉/保持器配置: 使能
        拉/保持器选择: 保持器
        上拉/下拉选择: 100K欧姆下拉(选择了保持器此配置无效)
        滞回器配置: 禁止 */



void lcdif_pin_config(void);
void CLOCK_InitVideoPll(void);

void lcdif_clock_init(void);
void BOARD_InitLcd(void);
void APP_ELCDIF_Init(void);
void ELCDIF_RgbModeInit(void);
void APP_FillFrameBuffer(uint32_t frameBuffer[APP_IMG_HEIGHT][APP_IMG_WIDTH]);

#endif
