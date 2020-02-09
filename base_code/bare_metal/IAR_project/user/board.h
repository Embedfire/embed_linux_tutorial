/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME "MCIMX6ULL-EVK"

/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE DEBUG_CONSOLE_DEVICE_TYPE_IUART
#define BOARD_DEBUG_UART_BAUDRATE 115200U
#define BOARD_DEBUG_UART_BASEADDR UART1_BASE

#define NXP74LV595_INPUT_STCP_GPIO GPIO5
#define NXP74LV595_INPUT_STCP_PIN 7U
#define NXP74LV595_INPUT_OE_GPIO GPIO5
#define NXP74LV595_INPUT_OE_PIN 8U
#define NXP74LV595_INPUT_SDI_GPIO GPIO5
#define NXP74LV595_INPUT_SDI_PIN 10U
#define NXP74LV595_INPUT_SHCP_GPIO GPIO5
#define NXP74LV595_INPUT_SHCP_PIN 11U

typedef enum _NXP74LV595_signal
{
    kSignal_NXP74LV595_Low = 0U,
    kSignal_NXP74LV595_High = 1U
} NXP74LV595_signal_t;

typedef enum _NXP74LV595_parOutputPins
{
    kNXP74LV595_HDMI_nRST = 0U,
    kNXP74LV595_ENET1_nRST = 1U,
    kNXP74LV595_ENET2_nRST = 2U,
    kNXP74LV595_CAN1_2_STBY = 3U,
    kNXP74LV595_BT_nPWD = 4U,
    kNXP74LV595_CSI_RST = 5U,
    kNXP74LV595_CSI_PWDN = 6U,
    kNXP74LV595_LCD_nPWREN = 7U
} NXP74LV595_parOutputPins_t;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
void BOARD_InitMemory(void);

uint32_t BOARD_DebugConsoleSrcFreq(void);

void BOARD_InitDebugConsole(void);

void BOARD_NXP74LV595_SetValue(NXP74LV595_parOutputPins_t pin, NXP74LV595_signal_t value);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
