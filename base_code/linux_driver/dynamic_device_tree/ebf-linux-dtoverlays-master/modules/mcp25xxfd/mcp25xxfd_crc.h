/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */
#ifndef __MCP25XXFD_CRC_H
#define __MCP25XXFD_CRC_H

#include "mcp25xxfd_priv.h"

int mcp25xxfd_crc_enable_int(struct mcp25xxfd_priv *priv, bool enable);
int mcp25xxfd_crc_clear_int(struct mcp25xxfd_priv *priv);

#endif /* __MCP25XXFD_CRC_H */
