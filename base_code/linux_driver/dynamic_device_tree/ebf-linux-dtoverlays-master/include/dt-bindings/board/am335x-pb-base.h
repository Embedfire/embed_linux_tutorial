/*
 * This header provides constants for PocketBeagle pinctrl bindings.
 *
 * Copyright (C) 2019 Jason Kridner <jdk@ti.com>
 */

#ifndef _DT_BINDINGS_BOARD_AM335X_PB_BASE_H
#define _DT_BINDINGS_BOARD_AM335X_PB_BASE_H


/************************/
/* P1 Header */
/************************/

/* P1_01                VIN-AC */

/* P1_02 (ZCZ ball R5) gpio2_23 */
#define PB_P1_02 0x08e4

/* P1_03 (ZCZ ball F15)  usb1_vbus_out         */

/* P1_04 (ZCZ ball R6) gpio2_25 */

#define PB_P1_04 0x08ec
/* P1_05 (ZCZ ball T18)  usb1_vbus_in         */

/* P1_06 (ZCZ ball A16) spi0_cs0 */
#define PB_P1_06 0x095c

/* P1_07                VIN-USB */

/* P1_08 (ZCZ ball A17) spi0_sclk */

#define PB_P1_08 0x0950
/* P1_09 (ZCZ ball R18)  USB1-DN         */

/* P1_10 (ZCZ ball B17) spi0_d0 */
#define PB_P1_10 0x0954

/* P1_11 (ZCZ ball R17)  USB1-DP         */

/* P1_12 (ZCZ ball B16) spi0_d1 */
#define PB_P1_12 0x0958

/* P1_13 (ZCZ ball P17)  USB1-ID         */

/* P1_14                VOUT-3.3V */

/* P1_15                GND */

/* P1_16                GND */

/* P1_17 (ZCZ ball A9)  VREFN         */

/* P1_18 (ZCZ ball B9)  VREFP         */

/* P1_19 (ZCZ ball B6)  AIN0         */

/* P1_20 (ZCZ ball D14) gpio0_20 */
#define PB_P1_20 0x09b4

/* P1_21 (ZCZ ball C7)  AIN1         */

/* P1_22                GND */

/* P1_23 (ZCZ ball B7)  AIN2         */

/* P1_24                VOUT-5V */

/* P1_25 (ZCZ ball A7)  AIN3         */

/* P1_26 (ZCZ ball D18) i2c2_sda */
#define PB_P1_26 0x0978

/* P1_27 (ZCZ ball C8)  AIN4         */

/* P1_28 (ZCZ ball D17) i2c2_scl */
#define PB_P1_28 0x097c

/* P1_29 (ZCZ ball A14) pru0_in7 */
#define PB_P1_29 0x09ac

/* P1_30 (ZCZ ball E16) uart0_txd */
#define PB_P1_30 0x0974

/* P1_31 (ZCZ ball B12) pru0_in4 */
#define PB_P1_31 0x09a0

/* P1_32 (ZCZ ball E15) uart0_rxd */
#define PB_P1_32 0x0970

/* P1_33 (ZCZ ball B13) pru0_in1 */
#define PB_P1_33 0x0994

/* P1_34 (ZCZ ball T11) gpio0_26 */
#define PB_P1_34 0x0828

/* P1_35 (ZCZ ball V5) pru1_in10 */
#define PB_P1_35 0x08e8

/* P1_36 (ZCZ ball A13) ehrpwm0a */
#define PB_P1_36 0x0990


/************************/
/* P2 Header */
/************************/

/* P2_01 (ZCZ ball U14) ehrpwm1a */
#define PB_P2_01 0x0848

/* P2_02 (ZCZ ball V17) gpio1_27 */
#define PB_P2_02 0x086c

/* P2_03 (ZCZ ball T10) gpio0_23 */
#define PB_P2_03 0x0824

/* P2_04 (ZCZ ball T16) gpio1_26 */
#define PB_P2_04 0x0868

/* P2_05 (ZCZ ball T17) uart4_rxd */
#define PB_P2_05 0x0870

/* P2_06 (ZCZ ball U16) gpio1_25 */
#define PB_P2_06 0x0864

/* P2_07 (ZCZ ball U17) uart4_txd */
#define PB_P2_07 0x0874

/* P2_08 (ZCZ ball U18) gpio1_28 */
#define PB_P2_08 0x0878

/* P2_09 (ZCZ ball D15) i2c1_scl */
#define PB_P2_09 0x0984

/* P2_10 (ZCZ ball R14) gpio1_20 */
#define PB_P2_10 0x0850

/* P2_11 (ZCZ ball D16) i2c1_sda */
#define PB_P2_11 0x0980

/* P2_12                POWER_BUTTON */

/* P2_13                VOUT-5V */

/* P2_14                BAT-VIN */

/* P2_15                GND */

/* P2_16                BAT-TEMP */

/* P2_17 (ZCZ ball V12) gpio2_1 */
#define PB_P2_17 0x088c

/* P2_18 (ZCZ ball U13) gpio1_15 */
#define PB_P2_18 0x083c

/* P2_19 (ZCZ ball U12) gpio0_27 */
#define PB_P2_19 0x082c

/* P2_20 (ZCZ ball T13) gpio2_0 */
#define PB_P2_20 0x0888

/* P2_21                GND */

/* P2_22 (ZCZ ball V13) gpio1_14 */
#define PB_P2_22 0x0838

/* P2_23                VOUT-3.3V */

/* P2_24 (ZCZ ball T12) gpio1_12 */
#define PB_P2_24 0x0830

/* P2_25 (ZCZ ball E17) spi1_d1 */
#define PB_P2_25 0x096c

/* P2_26                RESET# */

/* P2_27 (ZCZ ball E18) spi1_d0 */
#define PB_P2_27 0x0968

/* P2_28 (ZCZ ball D13) pru0_in6 */
#define PB_P2_28 0x09a8

/* P2_29 (ZCZ ball C18) spi1_sclk */
#define PB_P2_29 0x0964

/* P2_30 (ZCZ ball C12) pru0_in3 */
#define PB_P2_30 0x099c

/* P2_31 (ZCZ ball A15) spi1_cs1 */
#define PB_P2_31 0x09b0

/* P2_32 (ZCZ ball D12) pru0_in2 */
#define PB_P2_32 0x0998

/* P2_33 (ZCZ ball R12) gpio1_13 */
#define PB_P2_33 0x0834

/* P2_34 (ZCZ ball C13) pru0_in5 */
#define PB_P2_34 0x09a4

/* P2_35 (ZCZ ball U5) gpio2_22 */
#define PB_P2_35 0x08e0

/* P2_36 (ZCZ ball C9)  AIN7         */
#endif
