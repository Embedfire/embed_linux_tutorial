// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_crc.h"
#include "mcp25xxfd_regs.h"
#include "mcp25xxfd_priv.h"

int mcp25xxfd_crc_enable_int(struct mcp25xxfd_priv *priv, bool enable)
{
	u32 mask = MCP25XXFD_CRC_CRCERRIE | MCP25XXFD_CRC_FERRIE;

	priv->regs.crc &= ~mask;
	priv->regs.crc |= enable ? mask : 0;

	return mcp25xxfd_cmd_write_mask(priv->spi, MCP25XXFD_CRC,
					priv->regs.crc, mask);
}

int mcp25xxfd_crc_clear_int(struct mcp25xxfd_priv *priv)
{
	return mcp25xxfd_cmd_write_mask(priv->spi, MCP25XXFD_CRC, 0,
					MCP25XXFD_CRC_CRCERRIF |
					MCP25XXFD_CRC_FERRIF);
}
