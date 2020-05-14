/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */
#ifndef __MCP25XXFD_PRIV_H
#define __MCP25XXFD_PRIV_H

#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/gpio/driver.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_regs.h"

/* some defines for the driver */
#define DEVICE_NAME "mcp25xxfd"

enum mcp25xxfd_model {
	CAN_MCP2517FD	= 0x2517,
};

struct mcp25xxfd_can_priv;
struct mcp25xxfd_priv {
	struct spi_device *spi;
	struct clk *clk;
	struct gpio_chip gpio;
	struct mcp25xxfd_can_priv *cpriv;

	/* the actual model of the mcp25xxfd */
	enum mcp25xxfd_model model;

	/* full device name used for debugfs ant interrupts */
	char device_name[32];

	/* everything clock related */
	int clock_freq;
	struct {
		/* clock configuration */
		int clock_pll;
		int clock_div2;
		int clock_odiv;
	} config;

	/* lock for enabling/disabling the clock */
	struct mutex clk_user_lock;
	u32 clk_user_mask;
	u32 clk_sleep_mask;

	/* power related */
	struct regulator *power;

	/* the distinct spi_speeds to use for spi communication */
	u32 spi_setup_speed_hz;
	u32 spi_normal_speed_hz;
	u32 spi_use_speed_hz;

	/* spi-tx/rx buffers for efficient transfers
	 * used during setup and irq
	 */
	struct mutex spi_rxtx_lock; /* protects use of spi_tx/rx */
	u8 spi_tx[MCP25XXFD_SRAM_SIZE];
	u8 spi_rx[MCP25XXFD_SRAM_SIZE];

	/* configuration registers */
	struct {
		u32 osc;
		u32 iocon;
		u32 crc;
		u32 ecccon;
	} regs;

	/* debugfs related */
#if defined(CONFIG_DEBUG_FS)
	struct dentry *debugfs_dir;
	struct dentry *debugfs_regs_dir;
#endif
};

#endif /* __MCP25XXFD_PRIV_H */
