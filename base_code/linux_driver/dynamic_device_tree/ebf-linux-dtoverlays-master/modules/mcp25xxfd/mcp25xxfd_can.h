/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CAN_H
#define __MCP25XXFD_CAN_H

#include "mcp25xxfd_can_debugfs.h"
#include "mcp25xxfd_can_priv.h"
#include "mcp25xxfd_priv.h"
#include "mcp25xxfd_regs.h"

/* get the optimal controller target mode */
static inline
int mcp25xxfd_can_targetmode(struct mcp25xxfd_can_priv *cpriv)
{
	return (cpriv->can.dev->mtu == CAN_MTU) ?
		MCP25XXFD_CAN_CON_MODE_CAN2_0 : MCP25XXFD_CAN_CON_MODE_MIXED;
}

static inline
void mcp25xxfd_can_queue_frame(struct mcp25xxfd_can_priv *cpriv,
			       s32 fifo, u16 ts, bool is_rx)
{
	int idx = cpriv->fifos.submit_queue_count;

	cpriv->fifos.submit_queue[idx].fifo = fifo;
	cpriv->fifos.submit_queue[idx].ts = ts;
	cpriv->fifos.submit_queue[idx].is_rx = is_rx;

	MCP25XXFD_DEBUGFS_INCR(cpriv->fifos.submit_queue_count);
}

/* get the current controller mode */
int mcp25xxfd_can_get_mode(struct mcp25xxfd_priv *priv, u32 *reg);

/* to put us to sleep fully we need the CAN controller to enter sleep mode */
int mcp25xxfd_can_sleep_mode(struct mcp25xxfd_priv *priv);

/* switch controller mode */
int mcp25xxfd_can_switch_mode_no_wait(struct mcp25xxfd_priv *priv,
				      u32 *reg, int mode);
int mcp25xxfd_can_switch_mode(struct mcp25xxfd_priv *priv,
			      u32 *reg, int mode);

/* probe the can controller */
int mcp25xxfd_can_probe(struct mcp25xxfd_priv *priv);

/* setup and the can controller net interface */
int mcp25xxfd_can_setup(struct mcp25xxfd_priv *priv);
void mcp25xxfd_can_remove(struct mcp25xxfd_priv *priv);

#endif /* __MCP25XXFD_CAN_H */
