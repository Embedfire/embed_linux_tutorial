/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */
#ifndef __MCP25XXFD_INT_H
#define __MCP25XXFD_INT_H

#include "mcp25xxfd_priv.h"

int mcp25xxfd_int_clear(struct mcp25xxfd_priv *priv);
int mcp25xxfd_int_enable(struct mcp25xxfd_priv *priv, bool enable);

#endif /* __MCP25XXFD_INT_H */
