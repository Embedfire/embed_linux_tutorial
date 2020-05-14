/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */
#ifndef __MCP25XXFD_CAN_INT_H
#define __MCP25XXFD_CAN_INT_H

#include "mcp25xxfd_priv.h"

int mcp25xxfd_can_int_clear(struct mcp25xxfd_priv *priv);
int mcp25xxfd_can_int_enable(struct mcp25xxfd_priv *priv, bool enable);

irqreturn_t mcp25xxfd_can_int(int irq, void *dev_id);

#endif /* __MCP25XXFD_CAN_INT_H */
