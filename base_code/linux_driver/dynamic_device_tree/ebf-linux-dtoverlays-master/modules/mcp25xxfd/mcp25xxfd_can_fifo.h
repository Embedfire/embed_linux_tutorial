/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CAN_FIFO_H
#define __MCP25XXFD_CAN_FIFO_H

#include "mcp25xxfd_can_priv.h"

int mcp25xxfd_can_fifo_setup(struct mcp25xxfd_can_priv *cpriv);
void mcp25xxfd_can_fifo_release(struct mcp25xxfd_can_priv *cpriv);

#endif /* __MCP25XXFD_CAN_FIFO_H */
