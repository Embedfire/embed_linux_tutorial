// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 *
 * Based on Microchip MCP251x CAN controller driver written by
 * David Vrabel, Copyright 2006 Arcom Control Systems Ltd.
 */

#include <linux/can/core.h>
#include <linux/can/dev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_can.h"
#include "mcp25xxfd_can_id.h"
#include "mcp25xxfd_can_tx.h"
#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_regs.h"

/* mostly bit manipulations to move between stages */
static struct mcp25xxfd_tx_spi_message *
mcp25xxfd_can_tx_queue_first_spi_message(struct mcp25xxfd_tx_spi_message_queue *
					 queue, u32 *bitmap)
{
	u32 first = ffs(*bitmap);

	if (!first)
		return NULL;

	return queue->fifo2message[first - 1];
}

static void mcp25xxfd_can_tx_queue_remove_spi_message(u32 *bitmap, int fifo)
{
	*bitmap &= ~BIT(fifo);
}

static void mcp25xxfd_can_tx_queue_add_spi_message(u32 *bitmap, int fifo)
{
	*bitmap |= BIT(fifo);
}

static void mcp25xxfd_can_tx_queue_move_spi_message(u32 *src, u32 *dest,
						    int fifo)
{
	mcp25xxfd_can_tx_queue_remove_spi_message(src, fifo);
	mcp25xxfd_can_tx_queue_add_spi_message(dest, fifo);
}

static void mcp25xxfd_can_tx_spi_message_fill_fifo_complete(void *context)
{
	struct mcp25xxfd_tx_spi_message *msg = context;
	struct mcp25xxfd_can_priv *cpriv = msg->cpriv;
	struct mcp25xxfd_tx_spi_message_queue *q = cpriv->fifos.tx_queue;
	unsigned long flags;

	/* reset transfer length to without data (DLC = 0) */
	msg->fill_fifo.xfer.len = sizeof(msg->fill_fifo.data.cmd) +
		sizeof(msg->fill_fifo.data.header);

	/* we need to hold this lock to protect us from
	 * concurrent access by start_xmit
	 */
	spin_lock_irqsave(&cpriv->fifos.tx_queue->lock, flags);

	/* move to in_trigger_fifo_transfer */
	mcp25xxfd_can_tx_queue_move_spi_message(&q->in_fill_fifo_transfer,
						&q->in_trigger_fifo_transfer,
						msg->fifo);

	spin_unlock_irqrestore(&cpriv->fifos.tx_queue->lock, flags);
}

static void mcp25xxfd_can_tx_spi_message_trigger_fifo_complete(void *context)
{
	struct mcp25xxfd_tx_spi_message *msg = context;
	struct mcp25xxfd_can_priv *cpriv = msg->cpriv;
	struct mcp25xxfd_tx_spi_message_queue *q = cpriv->fifos.tx_queue;
	unsigned long flags;

	/* we need to hold this lock to protect us from
	 * concurrent access by the interrupt thread
	 */
	spin_lock_irqsave(&cpriv->fifos.tx_queue->lock, flags);

	/* move to can_transfer */
	mcp25xxfd_can_tx_queue_move_spi_message(&q->in_trigger_fifo_transfer,
						&q->in_can_transfer,
						msg->fifo);

	spin_unlock_irqrestore(&cpriv->fifos.tx_queue->lock, flags);
}

static
void mcp25xxfd_can_tx_message_init(struct mcp25xxfd_can_priv *cpriv,
				   struct mcp25xxfd_tx_spi_message *msg,
				   int fifo)
{
	const u32 trigger = MCP25XXFD_CAN_FIFOCON_TXREQ |
		MCP25XXFD_CAN_FIFOCON_UINC;
	const int first_byte = mcp25xxfd_cmd_first_byte(trigger);
	u32 addr;

	/* and initialize the structure */
	msg->cpriv = cpriv;
	msg->fifo = fifo;

	/* init fill_fifo */
	spi_message_init(&msg->fill_fifo.msg);
	msg->fill_fifo.msg.complete =
		mcp25xxfd_can_tx_spi_message_fill_fifo_complete;
	msg->fill_fifo.msg.context = msg;

	msg->fill_fifo.xfer.speed_hz = cpriv->priv->spi_use_speed_hz;
	msg->fill_fifo.xfer.tx_buf = msg->fill_fifo.data.cmd;
	msg->fill_fifo.xfer.len = sizeof(msg->fill_fifo.data.cmd) +
		sizeof(msg->fill_fifo.data.header);
	spi_message_add_tail(&msg->fill_fifo.xfer, &msg->fill_fifo.msg);

	addr = MCP25XXFD_SRAM_ADDR(cpriv->fifos.info[fifo].offset);
	mcp25xxfd_cmd_calc(MCP25XXFD_INSTRUCTION_WRITE, addr,
			   msg->fill_fifo.data.cmd);

	/* init trigger_fifo */
	spi_message_init(&msg->trigger_fifo.msg);
	msg->trigger_fifo.msg.complete =
		mcp25xxfd_can_tx_spi_message_trigger_fifo_complete;
	msg->trigger_fifo.msg.context = msg;

	msg->trigger_fifo.xfer.speed_hz = cpriv->priv->spi_use_speed_hz;
	msg->trigger_fifo.xfer.tx_buf = msg->trigger_fifo.data.cmd;
	msg->trigger_fifo.xfer.len = sizeof(msg->trigger_fifo.data.cmd) +
		sizeof(msg->trigger_fifo.data.data);
	spi_message_add_tail(&msg->trigger_fifo.xfer, &msg->trigger_fifo.msg);

	mcp25xxfd_cmd_calc(MCP25XXFD_INSTRUCTION_WRITE,
			   MCP25XXFD_CAN_FIFOCON(fifo) + first_byte,
			   msg->trigger_fifo.data.cmd);
	msg->trigger_fifo.data.data = trigger >> (8 * first_byte);

	/* and add to idle tx transfers */
	mcp25xxfd_can_tx_queue_add_spi_message(&cpriv->fifos.tx_queue->idle,
					       fifo);
}

static
void mcp25xxfd_can_tx_queue_manage_nolock(struct mcp25xxfd_can_priv *cpriv,
					  int state)
{
	struct net_device *net = cpriv->can.dev;

	/* skip early */
	if (state == cpriv->fifos.tx_queue->state)
		return;

	/* start/stop netif_queue if necessary */
	switch (cpriv->fifos.tx_queue->state) {
	case MCP25XXFD_CAN_TX_QUEUE_STATE_RUNABLE:
		switch (state) {
		case MCP25XXFD_CAN_TX_QUEUE_STATE_RESTART:
		case MCP25XXFD_CAN_TX_QUEUE_STATE_STARTED:
			netif_wake_queue(net);
			cpriv->fifos.tx_queue->state =
				MCP25XXFD_CAN_TX_QUEUE_STATE_STARTED;
			break;
		}
		break;
	case MCP25XXFD_CAN_TX_QUEUE_STATE_STOPPED:
		switch (state) {
		case MCP25XXFD_CAN_TX_QUEUE_STATE_STARTED:
			netif_wake_queue(net);
			cpriv->fifos.tx_queue->state = state;
			break;
		}
		break;
	case MCP25XXFD_CAN_TX_QUEUE_STATE_STARTED:
		switch (state) {
		case MCP25XXFD_CAN_TX_QUEUE_STATE_RUNABLE:
		case MCP25XXFD_CAN_TX_QUEUE_STATE_STOPPED:
			netif_stop_queue(net);
			cpriv->fifos.tx_queue->state = state;
			break;
		}
		break;
	default:
		WARN(true, "Unsupported tx_queue state: %i\n",
		     cpriv->fifos.tx_queue->state);
		break;
	}
}

void mcp25xxfd_can_tx_queue_manage(struct mcp25xxfd_can_priv *cpriv, int state)
{
	unsigned long flags;

	spin_lock_irqsave(&cpriv->fifos.tx_queue->lock, flags);

	mcp25xxfd_can_tx_queue_manage_nolock(cpriv, state);

	spin_unlock_irqrestore(&cpriv->fifos.tx_queue->lock, flags);
}

void mcp25xxfd_can_tx_queue_restart(struct mcp25xxfd_can_priv *cpriv)
{
	u32 state = MCP25XXFD_CAN_TX_QUEUE_STATE_RESTART;
	unsigned long flags;
	u32 mask;

	spin_lock_irqsave(&cpriv->fifos.tx_queue->lock, flags);

	/* only move if there is nothing pending or idle */
	mask = cpriv->fifos.tx_queue->idle |
		cpriv->fifos.tx_queue->in_fill_fifo_transfer |
		cpriv->fifos.tx_queue->in_trigger_fifo_transfer |
		cpriv->fifos.tx_queue->in_can_transfer;
	if (mask)
		goto out;

	/* move all items from transferred to idle */
	cpriv->fifos.tx_queue->idle |= cpriv->fifos.tx_queue->transferred;
	cpriv->fifos.tx_queue->transferred = 0;

	/* and enable queue */
	mcp25xxfd_can_tx_queue_manage_nolock(cpriv, state);
out:
	spin_unlock_irqrestore(&cpriv->fifos.tx_queue->lock, flags);
}

static
int mcp25xxfd_can_tx_tef_read(struct mcp25xxfd_can_priv *cpriv,
			      int start, int count)
{
	u32 tef_offset = start * cpriv->fifos.tef.size;
	struct mcp25xxfd_can_obj_tef *tef =
		(struct mcp25xxfd_can_obj_tef *)(cpriv->sram + tef_offset);
	int last, read, ret;

	/* compute how many we can read in one go */
	last = start + count;
	read = (last > cpriv->fifos.tef.count) ?
		(cpriv->fifos.tef.count - start) :
		count;

	/* and read it */
	ret = mcp25xxfd_cmd_read_regs(cpriv->priv->spi,
				      MCP25XXFD_SRAM_ADDR(tef_offset),
				      &tef->id, sizeof(*tef) * read);
	if (ret)
		return ret;

	/* and read a second part on wrap */
	if (read != count) {
		/* update stats */
		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tef_read_splits);
		/* compute the addresses  */
		read = count - read;
		tef = (struct mcp25xxfd_can_obj_tef *)(cpriv->sram);
		/* and read again */
		ret = mcp25xxfd_cmd_read_regs(cpriv->priv->spi,
					      MCP25XXFD_SRAM_ADDR(0),
					      &tef->id,
					      sizeof(*tef) * read);
	}

	return ret;
}

static
int mcp25xxfd_can_tx_handle_int_tefif_fifo(struct mcp25xxfd_can_priv *cpriv,
					   bool read_data)
{
	u32 tef_offset = cpriv->fifos.tef.index * cpriv->fifos.tef.size;
	struct mcp25xxfd_can_obj_tef *tef =
		(struct mcp25xxfd_can_obj_tef *)(cpriv->sram + tef_offset);
	int fifo, ret;
	unsigned long flags;

	/* read the next TEF entry to get the transmit timestamp and fifo */
	if (read_data) {
		ret = mcp25xxfd_can_tx_tef_read(cpriv,
						cpriv->fifos.tef.index, 1);
		if (ret)
			return ret;
	}

	/* get the fifo from tef */
	fifo = (tef->flags & MCP25XXFD_CAN_OBJ_FLAGS_SEQ_MASK) >>
		MCP25XXFD_CAN_OBJ_FLAGS_SEQ_SHIFT;

	/* check that the fifo is valid */
	spin_lock_irqsave(&cpriv->fifos.tx_queue->lock, flags);
	if ((cpriv->fifos.tx_queue->in_can_transfer & BIT(fifo)) == 0)
		netdev_err(cpriv->can.dev,
			   "tefif: fifo %i not pending - tef data: id: %08x flags: %08x, ts: %08x - this may be a problem with spi signal quality- try reducing spi-clock speed if this can get reproduced",
			   fifo, tef->id, tef->flags, tef->ts);
	spin_unlock_irqrestore(&cpriv->fifos.tx_queue->lock, flags);

	/* update stats */
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tef_reads);

	/* now we can schedule the fifo for echo submission */
	mcp25xxfd_can_queue_frame(cpriv, fifo, tef->ts, false);

	/* increment the tef index with wraparround */
	cpriv->fifos.tef.index++;
	if (cpriv->fifos.tef.index >= cpriv->fifos.tef.count)
		cpriv->fifos.tef.index = 0;

	/* finally just increment the TEF pointer */
	return mcp25xxfd_cmd_write_mask(cpriv->priv->spi, MCP25XXFD_CAN_TEFCON,
					MCP25XXFD_CAN_TEFCON_UINC,
					MCP25XXFD_CAN_TEFCON_UINC);
}

/* reading TEF entries can be made even more efficient by reading
 * multiple TEF entries in one go.
 * Under the assumption that we have count(TEF) >= count(TX_FIFO)
 * we can even release TEFs early (before we read them)
 * (and potentially restarting the transmit-queue early aswell)
 */

static int
mcp25xxfd_can_tx_handle_int_tefif_conservative(struct mcp25xxfd_can_priv *cpriv)
{
	u32 tefsta;
	int ret;

	/* read the TEF status */
	ret = mcp25xxfd_cmd_read_mask(cpriv->priv->spi, MCP25XXFD_CAN_TEFSTA,
				      &tefsta, MCP25XXFD_CAN_TEFSTA_TEFNEIF);
	if (ret)
		return ret;

	/* read the tef in an inefficient loop */
	while (tefsta & MCP25XXFD_CAN_TEFSTA_TEFNEIF) {
		/* read one tef */
		ret = mcp25xxfd_can_tx_handle_int_tefif_fifo(cpriv, true);
		if (ret)
			return ret;

		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tef_conservative_reads);

		/* read the TEF status */
		ret = mcp25xxfd_cmd_read_mask(cpriv->priv->spi,
					      MCP25XXFD_CAN_TEFSTA, &tefsta,
					      MCP25XXFD_CAN_TEFSTA_TEFNEIF);
		if (ret)
			return ret;
	}

	return 0;
}

static int
mcp25xxfd_can_tx_handle_int_tefif_optimized(struct mcp25xxfd_can_priv *cpriv,
					    u32 finished)
{
	int i, fifo, count, ret;

	/* count the number of fifos that have terminated */
	for (i = 0, fifo = cpriv->fifos.tx.start, count = 0;
	     i < cpriv->fifos.tx.count; i++, fifo++)
		if (finished & BIT(fifo))
			count++;

	/* read them in one go if possible
	 * we also assume that we have count(TEF) >= count(TX-FIFOS)
	 * this may require 2 reads when we wrap arround
	 * (that is unless count(TEF) == count(TX-FIFOS))
	 */
	ret = mcp25xxfd_can_tx_tef_read(cpriv, cpriv->fifos.tef.index, count);
	if (ret)
		return ret;

	/* update stats */
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tef_optimized_reads);
	i = min_t(int, MCP25XXFD_CAN_TEF_READ_BINS - 1, count - 1);
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tef_optimized_read_sizes[i]);

	/* now iterate those */
	for (i = 0, fifo = cpriv->fifos.tx.start; i < cpriv->fifos.tx.count;
	     i++, fifo++) {
		if (finished & BIT(fifo)) {
			ret = mcp25xxfd_can_tx_handle_int_tefif_fifo(cpriv,
								     false);
			if (ret)
				return ret;
		}
	}

	return 0;
}

int mcp25xxfd_can_tx_handle_int_tefif(struct mcp25xxfd_can_priv *cpriv)
{
	unsigned long flags;
	u32 finished;

	if (!(cpriv->status.intf & MCP25XXFD_CAN_INT_TEFIF))
		return 0;

	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_tef_count);

	spin_lock_irqsave(&cpriv->fifos.tx_queue->lock, flags);

	/* compute finished fifos and clear them immediately */
	finished = (cpriv->fifos.tx_queue->in_can_transfer ^
		    cpriv->status.txreq) &
		cpriv->fifos.tx_queue->in_can_transfer;

	spin_unlock_irqrestore(&cpriv->fifos.tx_queue->lock, flags);

	/* run in optimized mode if possible */
	if (finished)
		return mcp25xxfd_can_tx_handle_int_tefif_optimized(cpriv,
								   finished);
	/* otherwise play it safe */
	netdev_warn(cpriv->can.dev,
		    "Something is wrong - we got a TEF interrupt but we were not able to detect a finished fifo\n");
	return mcp25xxfd_can_tx_handle_int_tefif_conservative(cpriv);
}

static
void mcp25xxfd_can_tx_fill_fifo_common(struct mcp25xxfd_can_priv *cpriv,
				       struct mcp25xxfd_tx_spi_message *smsg,
				       struct mcp25xxfd_can_obj_tx *tx,
				       int dlc, u8 *data)
{
	int len = can_dlc2len(dlc);

	/* update statistics */
	MCP25XXFD_DEBUGFS_INCR(cpriv->fifos.tx.dlc_usage[dlc]);
	MCP25XXFD_DEBUGFS_INCR(cpriv->fifos.info[smsg->fifo].use_count);

	/* add fifo number as seq */
	tx->flags |= smsg->fifo << MCP25XXFD_CAN_OBJ_FLAGS_SEQ_SHIFT;

	/* copy data to tx->data for future reference */
	memcpy(tx->data, data, len);

	/* transform header to controller format */
	mcp25xxfd_cmd_convert_from_cpu(&tx->id, sizeof(*tx) / sizeof(u32));

	/* copy header + data to final location - we are not aligned */
	memcpy(smsg->fill_fifo.data.header, &tx->id, sizeof(*tx) + len);

	/* transfers to sram should be a multiple of 4 and be zero padded */
	for (; len & 3; len++)
		*(smsg->fill_fifo.data.header + sizeof(*tx) + len) = 0;

	/* convert it back to CPU format */
	mcp25xxfd_cmd_convert_to_cpu(&tx->id, sizeof(*tx) / sizeof(u32));

	/* set up size of transfer */
	smsg->fill_fifo.xfer.len = sizeof(smsg->fill_fifo.data.cmd) +
		sizeof(smsg->fill_fifo.data.header) + len;
}

static
void mcp25xxfd_can_tx_fill_fifo_fd(struct mcp25xxfd_can_priv *cpriv,
				   struct canfd_frame *frame,
				   struct mcp25xxfd_tx_spi_message *smsg,
				   struct mcp25xxfd_can_obj_tx *tx)
{
	int dlc = can_len2dlc(frame->len);

	/* update some statistics */
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tx_fd_count);

	/* compute can id */
	mcp25xxfd_can_id_to_mcp25xxfd(frame->can_id, &tx->id, &tx->flags);

	/* setup flags */
	tx->flags |= dlc << MCP25XXFD_CAN_OBJ_FLAGS_DLC_SHIFT;
	tx->flags |= (frame->can_id & CAN_EFF_FLAG) ?
		MCP25XXFD_CAN_OBJ_FLAGS_IDE : 0;
	tx->flags |= (frame->can_id & CAN_RTR_FLAG) ?
		MCP25XXFD_CAN_OBJ_FLAGS_RTR : 0;
	if (frame->flags & CANFD_BRS) {
		tx->flags |= MCP25XXFD_CAN_OBJ_FLAGS_BRS;
		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tx_brs_count);
	}
	tx->flags |= (frame->flags & CANFD_ESI) ?
		MCP25XXFD_CAN_OBJ_FLAGS_ESI : 0;
	tx->flags |= MCP25XXFD_CAN_OBJ_FLAGS_FDF;

	/* and do common processing */
	mcp25xxfd_can_tx_fill_fifo_common(cpriv, smsg, tx, dlc, frame->data);
}

static
void mcp25xxfd_can_tx_fill_fifo(struct mcp25xxfd_can_priv *cpriv,
				struct can_frame *frame,
				struct mcp25xxfd_tx_spi_message *smsg,
				struct mcp25xxfd_can_obj_tx *tx)
{
	/* set frame to valid dlc */
	if (frame->can_dlc > 8)
		frame->can_dlc = 8;

	/* compute can id */
	mcp25xxfd_can_id_to_mcp25xxfd(frame->can_id, &tx->id, &tx->flags);

	/* setup flags */
	tx->flags |= frame->can_dlc << MCP25XXFD_CAN_OBJ_FLAGS_DLC_SHIFT;
	tx->flags |= (frame->can_id & CAN_EFF_FLAG) ?
		MCP25XXFD_CAN_OBJ_FLAGS_IDE : 0;
	tx->flags |= (frame->can_id & CAN_RTR_FLAG) ?
		MCP25XXFD_CAN_OBJ_FLAGS_RTR : 0;

	/* and do common processing */
	mcp25xxfd_can_tx_fill_fifo_common(cpriv, smsg, tx, frame->can_dlc,
					  frame->data);
}

static struct mcp25xxfd_tx_spi_message *
mcp25xxfd_can_tx_queue_get_next_fifo(struct mcp25xxfd_can_priv *cpriv)
{
	u32 state = MCP25XXFD_CAN_TX_QUEUE_STATE_RUNABLE;
	struct mcp25xxfd_tx_spi_message_queue *q = cpriv->fifos.tx_queue;
	struct mcp25xxfd_tx_spi_message *smsg;
	unsigned long flags;

	/* we need to hold this lock to protect us against
	 * concurrent modifications of cpriv->fifos.tx_queue->idle
	 * in the interrupt thread
	 */
	spin_lock_irqsave(&q->lock, flags);

	/* get the first entry from idle */
	smsg = mcp25xxfd_can_tx_queue_first_spi_message(q, &q->idle);
	if (!smsg)
		goto out_busy;

	/* and move the fifo to next stage */
	mcp25xxfd_can_tx_queue_move_spi_message(&q->idle,
						&q->in_fill_fifo_transfer,
						smsg->fifo);

	/* if queue is empty then stop the network queue immediately */
	if (!q->idle)
		mcp25xxfd_can_tx_queue_manage_nolock(cpriv, state);
out_busy:
	spin_unlock_irqrestore(&q->lock, flags);

	return smsg;
}

/* submit the can message to the can-bus */
netdev_tx_t mcp25xxfd_can_tx_start_xmit(struct sk_buff *skb,
					struct net_device *net)
{
	u32 state = MCP25XXFD_CAN_TX_QUEUE_STATE_STOPPED;
	struct mcp25xxfd_can_priv *cpriv = netdev_priv(net);
	struct mcp25xxfd_tx_spi_message_queue *q = cpriv->fifos.tx_queue;
	struct mcp25xxfd_priv *priv = cpriv->priv;
	struct spi_device *spi = priv->spi;
	struct mcp25xxfd_tx_spi_message *smsg;
	struct mcp25xxfd_can_obj_tx *tx;
	unsigned long flags;
	int ret;

	/* invalid skb we can ignore */
	if (can_dropped_invalid_skb(net, skb))
		return NETDEV_TX_OK;

	/* acquire lock on spi so that we are are not risking
	 * some reordering of spi messages when we are running
	 * start_xmit in multiple threads (on multiple cores)
	 */
	spin_lock_irqsave(&q->spi_lock, flags);

	/* get the fifo message structure to process now */
	smsg = mcp25xxfd_can_tx_queue_get_next_fifo(cpriv);
	if (!smsg)
		goto out_busy;

	/* compute the fifo in sram */
	tx = (struct mcp25xxfd_can_obj_tx *)
		(cpriv->sram + cpriv->fifos.info[smsg->fifo].offset);

	/* fill in message from skb->data depending on can2.0 or canfd */
	if (can_is_canfd_skb(skb))
		mcp25xxfd_can_tx_fill_fifo_fd(cpriv,
					      (struct canfd_frame *)skb->data,
					      smsg, tx);
	else
		mcp25xxfd_can_tx_fill_fifo(cpriv,
					   (struct can_frame *)skb->data,
					   smsg, tx);

	/* submit the two messages asyncronously
	 * the reason why we separate transfers into two spi_messages is:
	 *  * because the spi framework (currently) does add a 10us delay
	 *    between 2 spi_transfers in a single spi_message when
	 *    change_cs is set - 2 consecutive spi messages show a shorter
	 *    cs disable phase increasing bus utilization
	 *    (code reduction with a fix in spi core would be aprox.50 lines)
	 *  * this allows the interrupt handler to start spi messages earlier
	 *    so reducing latencies a bit and to allow for better concurrency
	 *  * this separation - in the future - may get used to fill fifos
	 *    early and reduce the delay on "rollover"
	 */
	ret = spi_async(spi, &smsg->fill_fifo.msg);
	if (ret)
		goto out_async_failed;
	ret = spi_async(spi, &smsg->trigger_fifo.msg);
	if (ret)
		goto out_async_failed;

	/* unlock the spi bus */
	spin_unlock_irqrestore(&q->spi_lock, flags);

	/* keep it for reference until the message really got transmitted */
	can_put_echo_skb(skb, net, smsg->fifo);

	return NETDEV_TX_OK;
out_async_failed:
	netdev_err(net, "spi_async submission of fifo %i failed - %i\n",
		   smsg->fifo, ret);

out_busy:
	/* stop the queue */
	mcp25xxfd_can_tx_queue_manage_nolock(cpriv, state);

	spin_unlock_irqrestore(&q->spi_lock, flags);

	return NETDEV_TX_BUSY;
}

/* submit the fifo back to the network stack */
int mcp25xxfd_can_tx_submit_frame(struct mcp25xxfd_can_priv *cpriv, int fifo)
{
	struct mcp25xxfd_tx_spi_message_queue *q = cpriv->fifos.tx_queue;
	struct mcp25xxfd_can_obj_tx *tx = (struct mcp25xxfd_can_obj_tx *)
		(cpriv->sram + cpriv->fifos.info[fifo].offset);
	int dlc = (tx->flags & MCP25XXFD_CAN_OBJ_FLAGS_DLC_MASK) >>
		MCP25XXFD_CAN_OBJ_FLAGS_DLC_SHIFT;
	unsigned long flags;

	/* update counters */
	cpriv->can.dev->stats.tx_packets++;
	cpriv->can.dev->stats.tx_bytes += can_dlc2len(dlc);
	MCP25XXFD_DEBUGFS_INCR(cpriv->fifos.tx.dlc_usage[dlc]);
	if (tx->flags & MCP25XXFD_CAN_OBJ_FLAGS_FDF)
		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tx_fd_count);
	if (tx->flags & MCP25XXFD_CAN_OBJ_FLAGS_BRS)
		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, tx_brs_count);

	spin_lock_irqsave(&cpriv->fifos.tx_queue->lock, flags);

	/* release the echo buffer */
	can_get_echo_skb(cpriv->can.dev, fifo);

	/* move from in_can_transfer to transferred */
	mcp25xxfd_can_tx_queue_move_spi_message(&q->in_can_transfer,
						&q->transferred, fifo);

	spin_unlock_irqrestore(&cpriv->fifos.tx_queue->lock, flags);

	return 0;
}

/* interrupt handler */
int mcp25xxfd_can_tx_handle_int_txatif_fifo(struct mcp25xxfd_can_priv *cpriv,
					    int fifo)
{
	struct mcp25xxfd_tx_spi_message_queue *q = cpriv->fifos.tx_queue;
	u32 val;
	unsigned long flags;
	int ret;

	/* read fifo status */
	ret = mcp25xxfd_cmd_read(cpriv->priv->spi,
				 MCP25XXFD_CAN_FIFOSTA(fifo), &val);
	if (ret)
		return ret;

	/* clear the relevant interrupt flags */
	ret = mcp25xxfd_cmd_write_mask(cpriv->priv->spi,
				       MCP25XXFD_CAN_FIFOSTA(fifo), 0,
				       MCP25XXFD_CAN_FIFOSTA_TXABT |
				       MCP25XXFD_CAN_FIFOSTA_TXLARB |
				       MCP25XXFD_CAN_FIFOSTA_TXERR |
				       MCP25XXFD_CAN_FIFOSTA_TXATIF);
	if (ret)
		return ret;

	spin_lock_irqsave(&q->lock, flags);
	/* for specific cases we probably could trigger a retransmit
	 * instead of an abort.
	 */

	/* and we release it from the echo_skb buffer
	 * NOTE: this is one place where packet delivery will not
	 * be ordered, as we do not have any timing information
	 * when this occurred
	 */
	can_get_echo_skb(cpriv->can.dev, fifo);

	mcp25xxfd_can_tx_queue_move_spi_message(&q->in_can_transfer,
						&q->transferred, fifo);

	spin_unlock_irqrestore(&q->lock, flags);

	/* but we need to run a bit of cleanup */
	cpriv->status.txif &= ~BIT(fifo);
	cpriv->can.dev->stats.tx_aborted_errors++;

	/* handle all the known cases accordingly - ignoring FIFO full */
	val &= MCP25XXFD_CAN_FIFOSTA_TXABT |
		MCP25XXFD_CAN_FIFOSTA_TXLARB |
		MCP25XXFD_CAN_FIFOSTA_TXERR;
	switch (val) {
	case MCP25XXFD_CAN_FIFOSTA_TXERR:
		/* this indicates a possible bus error */
		break;
	default:
		dev_warn_ratelimited(&cpriv->priv->spi->dev,
				     "Unknown TX-Fifo abort condition: %08x - stopping tx-queue\n",
				     val);
		break;
	}

	return 0;
}

int mcp25xxfd_can_tx_handle_int_txatif(struct mcp25xxfd_can_priv *cpriv)
{
	int i, f, ret;

	/* if txatif is unset, then there are no
	 * can frames that have been transmitted
	 * and need to get reingested into the network stack
	 */
	if (!cpriv->status.txatif)
		return 0;
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_txat_count);

	/* process all the fifos with that flag set */
	for (i = 0, f = cpriv->fifos.tx.start; i < cpriv->fifos.tx.count;
	     i++, f++) {
		if (cpriv->status.txatif & BIT(f)) {
			ret = mcp25xxfd_can_tx_handle_int_txatif_fifo(cpriv, f);
			if (ret)
				return ret;
		}
	}

	return 0;
}

int mcp25xxfd_can_tx_queue_alloc(struct mcp25xxfd_can_priv *cpriv)
{
	struct mcp25xxfd_tx_spi_message *msg;
	size_t size = sizeof(struct mcp25xxfd_tx_spi_message_queue) +
		cpriv->fifos.tx.count * sizeof(*msg);
	int i, f;

	/* allocate the fifos as an array */
	cpriv->fifos.tx_queue = kzalloc(size, GFP_KERNEL);
	if (!cpriv->fifos.tx_queue)
		return -ENOMEM;

	/* initialize the tx_queue structure */
	spin_lock_init(&cpriv->fifos.tx_queue->lock);
	spin_lock_init(&cpriv->fifos.tx_queue->spi_lock);

	/* initialize the individual spi_message structures */
	for (i = 0, f = cpriv->fifos.tx.start; i < cpriv->fifos.tx.count;
	     i++, f++) {
		msg = &cpriv->fifos.tx_queue->message[i];
		cpriv->fifos.tx_queue->fifo2message[f] = msg;
		mcp25xxfd_can_tx_message_init(cpriv, msg, f);
	}

	return 0;
}

void mcp25xxfd_can_tx_queue_free(struct mcp25xxfd_can_priv *cpriv)
{
	/* eventually we may need to wait here
	 * for all transfers to have finished
	 */

	kfree(cpriv->fifos.tx_queue);
	cpriv->fifos.tx_queue = NULL;
}
