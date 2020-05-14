/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CLOCK_H
#define __MCP25XXFD_CLOCK_H

#include "mcp25xxfd_priv.h"

#define MCP25XXFD_CLK_USER_CAN BIT(0)
#define MCP25XXFD_CLK_USER_GPIO0 BIT(1)
#define MCP25XXFD_CLK_USER_GPIO1 BIT(2)
#define MCP25XXFD_CLK_USER_CLKOUT BIT(3)

/* shared (internal) clock control */
int mcp25xxfd_clock_init(struct mcp25xxfd_priv *priv);
int mcp25xxfd_clock_probe(struct mcp25xxfd_priv *priv);
void mcp25xxfd_clock_release(struct mcp25xxfd_priv *priv);

int mcp25xxfd_clock_stop(struct mcp25xxfd_priv *priv, int requestor_mask);
int mcp25xxfd_clock_start(struct mcp25xxfd_priv *priv, int requestor_mask);

void mcp25xxfd_clock_fake_sleep(struct mcp25xxfd_priv *priv);

#endif /* __MCP25XXFD_CLOCK_H */
