/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CAN_RX_H
#define __MCP25XXFD_CAN_RX_H

#include "mcp25xxfd_priv.h"

int mcp25xxfd_can_rx_submit_frame(struct mcp25xxfd_can_priv *cpriv, int fifo);

int mcp25xxfd_can_rx_handle_int_rxif(struct mcp25xxfd_can_priv *cpriv);
int mcp25xxfd_can_rx_handle_int_rxovif(struct mcp25xxfd_can_priv *cpriv);

#endif /* __MCP25XXFD_CAN_RX_H */
