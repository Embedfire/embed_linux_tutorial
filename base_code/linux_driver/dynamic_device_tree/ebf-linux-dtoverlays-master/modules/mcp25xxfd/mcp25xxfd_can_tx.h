/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CAN_TX_H
#define __MCP25XXFD_CAN_TX_H

#include <linux/spinlock.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_can_priv.h"

/* structure of a spi message that is prepared and can get submitted quickly */
struct mcp25xxfd_tx_spi_message {
	/* the network device this is related to */
	struct mcp25xxfd_can_priv *cpriv;
	/* the fifo this fills */
	u32 fifo;
	/* the xfer to fill in the fifo data */
	struct {
		struct spi_message msg;
		struct spi_transfer xfer;
		struct {
			u8 cmd[2];
			u8 header[sizeof(struct mcp25xxfd_can_obj_tx)];
			u8 data[64];
		} data;
	} fill_fifo;
	/* the xfer to enable transmission on the can bus */
	struct {
		struct spi_message msg;
		struct spi_transfer xfer;
		struct {
			u8 cmd[2];
			u8 data;
		} data;
	} trigger_fifo;
};

struct mcp25xxfd_tx_spi_message_queue {
	/* spinlock protecting the bitmaps
	 * as well as state and the skb_echo_* functions
	 */
	spinlock_t lock;
	/* bitmap of which fifo is in which stage */
	u32 idle;
	u32 in_fill_fifo_transfer;
	u32 in_trigger_fifo_transfer;
	u32 in_can_transfer;
	u32 transferred;

	/* the queue state as seen per controller */
	int state;
#define MCP25XXFD_CAN_TX_QUEUE_STATE_STOPPED 0
#define MCP25XXFD_CAN_TX_QUEUE_STATE_STARTED 1
#define MCP25XXFD_CAN_TX_QUEUE_STATE_RUNABLE 2
#define MCP25XXFD_CAN_TX_QUEUE_STATE_RESTART 3

	/* spinlock protecting spi submission order */
	spinlock_t spi_lock;

	/* map each fifo to a mcp25xxfd_tx_spi_message */
	struct mcp25xxfd_tx_spi_message *fifo2message[32];

	/* the individual messages */
	struct mcp25xxfd_tx_spi_message message[];
};

int mcp25xxfd_can_tx_submit_frame(struct mcp25xxfd_can_priv *cpriv, int fifo);
void mcp25xxfd_can_tx_queue_restart(struct mcp25xxfd_can_priv *cpriv);

int mcp25xxfd_can_tx_handle_int_txatif(struct mcp25xxfd_can_priv *cpriv);
int mcp25xxfd_can_tx_handle_int_tefif(struct mcp25xxfd_can_priv *cpriv);

netdev_tx_t mcp25xxfd_can_tx_start_xmit(struct sk_buff *skb,
					struct net_device *net);

void mcp25xxfd_can_tx_queue_manage(struct mcp25xxfd_can_priv *cpriv, int state);

int mcp25xxfd_can_tx_queue_alloc(struct mcp25xxfd_can_priv *cpriv);
void mcp25xxfd_can_tx_queue_free(struct mcp25xxfd_can_priv *cpriv);

#endif /* __MCP25XXFD_CAN_TX_H */
