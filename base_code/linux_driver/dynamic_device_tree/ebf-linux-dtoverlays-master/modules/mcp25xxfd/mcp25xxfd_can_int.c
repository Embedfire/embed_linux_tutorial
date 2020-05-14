// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#include <linux/can/core.h>
#include <linux/can/dev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sort.h>

#include "mcp25xxfd_regs.h"
#include "mcp25xxfd_can.h"
#include "mcp25xxfd_can_debugfs.h"
#include "mcp25xxfd_can_priv.h"
#include "mcp25xxfd_can_rx.h"
#include "mcp25xxfd_can_tx.h"
#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_ecc.h"

unsigned int reschedule_int_thread_after = 4;
module_param(reschedule_int_thread_after, uint, 0664);
MODULE_PARM_DESC(reschedule_int_thread_after,
		 "Reschedule the interrupt thread after this many loops\n");

static void mcp25xxfd_can_int_send_error_skb(struct mcp25xxfd_can_priv *cpriv)
{
	struct net_device *net = cpriv->can.dev;
	struct sk_buff *skb;
	struct can_frame *frame;

	/* allocate error frame */
	skb = alloc_can_err_skb(net, &frame);
	if (!skb) {
		netdev_err(net, "cannot allocate error skb\n");
		return;
	}

	/* setup can error frame data */
	frame->can_id |= cpriv->error_frame.id;
	memcpy(frame->data, cpriv->error_frame.data, sizeof(frame->data));

	/* and submit it */
	netif_receive_skb(skb);
}

static int mcp25xxfd_can_int_compare_obj_ts(const void *a, const void *b)
{
	s32 ats = ((struct mcp25xxfd_obj_ts *)a)->ts;
	s32 bts = ((struct mcp25xxfd_obj_ts *)b)->ts;

	if (ats < bts)
		return -1;
	if (ats > bts)
		return 1;
	return 0;
}

static int mcp25xxfd_can_int_submit_frames(struct mcp25xxfd_can_priv *cpriv)
{
	struct mcp25xxfd_obj_ts *queue = cpriv->fifos.submit_queue;
	int count = cpriv->fifos.submit_queue_count;
	int i, fifo;
	int ret;

	/* skip processing if the queue count is 0 */
	if (count == 0)
		goto out;

	/* sort the fifos (rx and tx - actually TEF) by receive timestamp */
	sort(queue, count, sizeof(*queue),
	     mcp25xxfd_can_int_compare_obj_ts, NULL);

	/* now submit the fifos  */
	for (i = 0; i < count; i++) {
		fifo = queue[i].fifo;
		ret = (queue[i].is_rx) ?
			mcp25xxfd_can_rx_submit_frame(cpriv, fifo) :
			mcp25xxfd_can_tx_submit_frame(cpriv, fifo);
		if (ret)
			return ret;
	}

	/* if we have received or transmitted something
	 * and the IVMIE is disabled, then enable it
	 * this is mostly to avoid unnecessary interrupts during a
	 * disconnected CAN BUS
	 */
	if (!(cpriv->status.intf | MCP25XXFD_CAN_INT_IVMIE)) {
		cpriv->status.intf |= MCP25XXFD_CAN_INT_IVMIE;
		ret = mcp25xxfd_cmd_write_mask(cpriv->priv->spi,
					       MCP25XXFD_CAN_INT,
					       cpriv->status.intf,
					       MCP25XXFD_CAN_INT_IVMIE);
		if (ret)
			return ret;
	}

out:
	/* enable tx_queue if necessary */
	mcp25xxfd_can_tx_queue_restart(cpriv);

	return 0;
}

static int mcp25xxfd_can_int_clear_int_flags(struct mcp25xxfd_can_priv *cpriv)
{
	u32 clearable_irq_active = cpriv->status.intf &
		MCP25XXFD_CAN_INT_IF_CLEAR_MASK;
	u32 clear_irq = cpriv->status.intf & (~MCP25XXFD_CAN_INT_IF_CLEAR_MASK);

	/* if no clearable flags are set then skip the whole transfer */
	if (!clearable_irq_active)
		return 0;

	return mcp25xxfd_cmd_write_mask(cpriv->priv->spi, MCP25XXFD_CAN_INT,
					clear_irq, clearable_irq_active);
}

static
int mcp25xxfd_can_int_handle_serrif_txmab(struct mcp25xxfd_can_priv *cpriv)
{
	int mode = mcp25xxfd_can_targetmode(cpriv);

	cpriv->can.dev->stats.tx_fifo_errors++;
	cpriv->can.dev->stats.tx_errors++;
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_serr_tx_count);

	/* data7 contains custom mcp25xxfd error flags */
	cpriv->error_frame.data[7] |= MCP25XXFD_CAN_ERR_DATA7_MCP25XXFD_SERR_TX;

	/* and switch back into the correct mode */
	return mcp25xxfd_can_switch_mode_no_wait(cpriv->priv,
						 &cpriv->regs.con, mode);
}

static
int mcp25xxfd_can_int_handle_serrif_rxmab(struct mcp25xxfd_can_priv *cpriv)
{
	cpriv->can.dev->stats.rx_dropped++;
	cpriv->can.dev->stats.rx_errors++;
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_serr_rx_count);

	/* data7 contains custom mcp25xxfd error flags */
	cpriv->error_frame.data[7] |= MCP25XXFD_CAN_ERR_DATA7_MCP25XXFD_SERR_RX;

	return 0;
}

static int mcp25xxfd_can_int_handle_serrif(struct mcp25xxfd_can_priv *cpriv)
{
	if (!(cpriv->status.intf & MCP25XXFD_CAN_INT_SERRIF))
		return 0;

	/* increment statistics counter now */
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_serr_count);

	/* interrupt flags have been cleared already */

	/* Errors here are:
	 * * Bus Bandwidth Error: when a RX Message Assembly Buffer
	 *   is still full when the next message has already arrived
	 *   the recived message shall be ignored
	 * * TX MAB Underflow: when a TX Message is invalid
	 *   due to ECC errors or TXMAB underflow
	 *   in this situatioon the system will transition to
	 *   Restricted or Listen Only mode
	 */

	cpriv->error_frame.id |= CAN_ERR_CRTL;
	cpriv->error_frame.data[1] |= CAN_ERR_CRTL_UNSPEC;

	/* a mode change + invalid message would indicate
	 * TX MAB Underflow
	 */
	if ((cpriv->status.intf & MCP25XXFD_CAN_INT_MODIF) &&
	    (cpriv->status.intf & MCP25XXFD_CAN_INT_IVMIF)) {
		return mcp25xxfd_can_int_handle_serrif_txmab(cpriv);
	}

	/* for RX there is only the RXIF an indicator
	 * - surprizingly RX-MAB does not change mode or anything
	 */
	if (cpriv->status.intf & MCP25XXFD_CAN_INT_RXIF)
		return mcp25xxfd_can_int_handle_serrif_rxmab(cpriv);

	/* the final case */
	dev_warn_ratelimited(&cpriv->priv->spi->dev,
			     "unidentified system interrupt - intf =  %08x\n",
			     cpriv->status.intf);

	return 0;
}

static int mcp25xxfd_can_int_handle_modif(struct mcp25xxfd_can_priv *cpriv)
{
	struct spi_device *spi = cpriv->priv->spi;
	int mode;
	int ret;

	/* Note that this irq does not get triggered in all situations
	 * for example SERRIF will move to RESTICTED or LISTENONLY
	 * but MODIF will not be raised!
	 */

	if (!(cpriv->status.intf & MCP25XXFD_CAN_INT_MODIF))
		return 0;
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_mod_count);

	/* get the current mode */
	ret = mcp25xxfd_can_get_mode(cpriv->priv, &mode);
	if (ret)
		return ret;
	mode = ret;

	/* switches to the same mode as before are ignored
	 * - this typically happens if the driver is shortly
	 *   switching to a different mode and then returning to the
	 *   original mode
	 */
	if (mode == cpriv->mode)
		return 0;

	/* if we are restricted, then return to "normal" mode */
	if (mode == MCP25XXFD_CAN_CON_MODE_RESTRICTED) {
		cpriv->mode = mode;
		mode = mcp25xxfd_can_targetmode(cpriv);
		return mcp25xxfd_can_switch_mode_no_wait(cpriv->priv,
							 &cpriv->regs.con,
							 mode);
	}

	/* the controller itself will transition to sleep, so we ignore it */
	if (mode == MCP25XXFD_CAN_CON_MODE_SLEEP) {
		cpriv->mode = mode;
		return 0;
	}

	/* these we need to handle correctly, so warn and give context */
	dev_warn(&spi->dev,
		 "Controller unexpectedly switched from mode %u to %u\n",
		 cpriv->mode, mode);

	/* assign the mode as current */
	cpriv->mode = mode;

	return 0;
}

static int mcp25xxfd_can_int_handle_eccif(struct mcp25xxfd_can_priv *cpriv)
{
	if (!(cpriv->status.intf & MCP25XXFD_CAN_INT_ECCIF))
		return 0;

	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_ecc_count);

	/* and prepare ERROR FRAME */
	cpriv->error_frame.id |= CAN_ERR_CRTL;
	cpriv->error_frame.data[1] |= CAN_ERR_CRTL_UNSPEC;
	/* data7 contains custom mcp25xxfd error flags */
	cpriv->error_frame.data[7] |= MCP25XXFD_CAN_ERR_DATA7_MCP25XXFD_ECC;

	/* delegate to interrupt cleaning */
	return mcp25xxfd_ecc_clear_int(cpriv->priv);
}

static void mcp25xxfd_can_int_handle_ivmif_tx(struct mcp25xxfd_can_priv *cpriv,
					      u32 *mask)
{
	/* check if it is really a known tx error */
	if ((cpriv->bus.bdiag[1] &
	     (MCP25XXFD_CAN_BDIAG1_DBIT1ERR |
	      MCP25XXFD_CAN_BDIAG1_DBIT0ERR |
	      MCP25XXFD_CAN_BDIAG1_NACKERR |
	      MCP25XXFD_CAN_BDIAG1_NBIT1ERR |
	      MCP25XXFD_CAN_BDIAG1_NBIT0ERR
		     )) == 0)
		return;

	/* mark it as a protocol error */
	cpriv->error_frame.id |= CAN_ERR_PROT;

	/* and update statistics */
	cpriv->can.dev->stats.tx_errors++;

	/* and handle all the known cases */
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_NACKERR) {
		/* TX-Frame not acknowledged - connected to CAN-bus? */
		*mask |= MCP25XXFD_CAN_BDIAG1_NACKERR;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_TX;
		cpriv->can.dev->stats.tx_aborted_errors++;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_NBIT1ERR) {
		/* TX-Frame CAN-BUS Level is unexpectedly dominant */
		*mask |= MCP25XXFD_CAN_BDIAG1_NBIT1ERR;
		cpriv->can.dev->stats.tx_carrier_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_BIT1;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_NBIT0ERR) {
		/* TX-Frame CAN-BUS Level is unexpectedly recessive */
		*mask |= MCP25XXFD_CAN_BDIAG1_NBIT0ERR;
		cpriv->can.dev->stats.tx_carrier_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_BIT0;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_DBIT1ERR) {
		/* TX-Frame CAN-BUS Level is unexpectedly dominant
		 * during data phase
		 */
		*mask |= MCP25XXFD_CAN_BDIAG1_DBIT1ERR;
		cpriv->can.dev->stats.tx_carrier_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_BIT1;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_DBIT0ERR) {
		/* TX-Frame CAN-BUS Level is unexpectedly recessive
		 * during data phase
		 */
		*mask |= MCP25XXFD_CAN_BDIAG1_DBIT0ERR;
		cpriv->can.dev->stats.tx_carrier_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_BIT0;
	}
}

static void mcp25xxfd_can_int_handle_ivmif_rx(struct mcp25xxfd_can_priv *cpriv,
					      u32 *mask)
{
	/* check if it is really a known tx error */
	if ((cpriv->bus.bdiag[1] &
	     (MCP25XXFD_CAN_BDIAG1_DCRCERR |
	      MCP25XXFD_CAN_BDIAG1_DSTUFERR |
	      MCP25XXFD_CAN_BDIAG1_DFORMERR |
	      MCP25XXFD_CAN_BDIAG1_NCRCERR |
	      MCP25XXFD_CAN_BDIAG1_NSTUFERR |
	      MCP25XXFD_CAN_BDIAG1_NFORMERR
		     )) == 0)
		return;

	/* mark it as a protocol error */
	cpriv->error_frame.id |= CAN_ERR_PROT;

	/* and update statistics */
	cpriv->can.dev->stats.rx_errors++;

	/* handle the cases */
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_DCRCERR) {
		/* RX-Frame with bad CRC during data phase */
		*mask |= MCP25XXFD_CAN_BDIAG1_DCRCERR;
		cpriv->can.dev->stats.rx_crc_errors++;
		cpriv->error_frame.data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_DSTUFERR) {
		/* RX-Frame with bad stuffing during data phase */
		*mask |= MCP25XXFD_CAN_BDIAG1_DSTUFERR;
		cpriv->can.dev->stats.rx_frame_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_STUFF;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_DFORMERR) {
		/* RX-Frame with bad format during data phase */
		*mask |= MCP25XXFD_CAN_BDIAG1_DFORMERR;
		cpriv->can.dev->stats.rx_frame_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_FORM;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_NCRCERR) {
		/* RX-Frame with bad CRC during data phase */
		*mask |= MCP25XXFD_CAN_BDIAG1_NCRCERR;
		cpriv->can.dev->stats.rx_crc_errors++;
		cpriv->error_frame.data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_NSTUFERR) {
		/* RX-Frame with bad stuffing during data phase */
		*mask |= MCP25XXFD_CAN_BDIAG1_NSTUFERR;
		cpriv->can.dev->stats.rx_frame_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_STUFF;
	}
	if (cpriv->bus.bdiag[1] & MCP25XXFD_CAN_BDIAG1_NFORMERR) {
		/* RX-Frame with bad format during data phase */
		*mask |= MCP25XXFD_CAN_BDIAG1_NFORMERR;
		cpriv->can.dev->stats.rx_frame_errors++;
		cpriv->error_frame.data[2] |= CAN_ERR_PROT_FORM;
	}
}

static int mcp25xxfd_can_int_handle_ivmif(struct mcp25xxfd_can_priv *cpriv)
{
	struct spi_device *spi = cpriv->priv->spi;
	u32 mask, bdiag1;
	int ret;

	if (!(cpriv->status.intf & MCP25XXFD_CAN_INT_IVMIF))
		return 0;

	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_ivm_count);

	/* if we have a systemerror as well,
	 * then ignore it as they correlate
	 */
	if (cpriv->status.intf & MCP25XXFD_CAN_INT_SERRIF)
		return 0;

	/* read bus diagnostics */
	ret = mcp25xxfd_cmd_read_regs(spi, MCP25XXFD_CAN_BDIAG0,
				      cpriv->bus.bdiag,
				      sizeof(cpriv->bus.bdiag));
	if (ret)
		return ret;

	/* clear the masks of bits to clear */
	mask = 0;

	/* check rx and tx errors */
	mcp25xxfd_can_int_handle_ivmif_tx(cpriv, &mask);
	mcp25xxfd_can_int_handle_ivmif_rx(cpriv, &mask);

	/* clear flags if we have bits masked */
	if (!mask) {
		/* the unsupported case, where we are not
		 * clearing any registers
		 */
		dev_warn_once(&spi->dev,
			      "found IVMIF situation not supported by driver - bdiag = [0x%08x, 0x%08x]",
			      cpriv->bus.bdiag[0], cpriv->bus.bdiag[1]);
		return -EINVAL;
	}

	/* clear the bits in bdiag1 */
	bdiag1 = cpriv->bus.bdiag[1] & (~mask);
	/* and write it */
	ret = mcp25xxfd_cmd_write_mask(spi, MCP25XXFD_CAN_BDIAG1, bdiag1, mask);
	if (ret)
		return ret;

	/* and clear the interrupt flag until we have received or transmited */
	cpriv->status.intf &= ~(MCP25XXFD_CAN_INT_IVMIE);
	return mcp25xxfd_cmd_write_mask(spi, MCP25XXFD_CAN_INT,
					cpriv->status.intf,
					MCP25XXFD_CAN_INT_IVMIE);
}

static int mcp25xxfd_can_int_handle_cerrif(struct mcp25xxfd_can_priv *cpriv)
{
	if (!(cpriv->status.intf & MCP25XXFD_CAN_INT_CERRIF))
		return 0;

	/* this interrupt exists primarilly to counter possible
	 * bus off situations more detailed information
	 * can be found and controlled in the TREC register
	 */

	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_cerr_count);

	netdev_warn(cpriv->can.dev, "CAN Bus error experienced");

	return 0;
}

static int mcp25xxfd_can_int_error_counters(struct mcp25xxfd_can_priv *cpriv)
{
	if (cpriv->status.trec & MCP25XXFD_CAN_TREC_TXWARN) {
		cpriv->bus.new_state = CAN_STATE_ERROR_WARNING;
		cpriv->error_frame.id |= CAN_ERR_CRTL;
		cpriv->error_frame.data[1] |= CAN_ERR_CRTL_TX_WARNING;
	}
	if (cpriv->status.trec & MCP25XXFD_CAN_TREC_RXWARN) {
		cpriv->bus.new_state = CAN_STATE_ERROR_WARNING;
		cpriv->error_frame.id |= CAN_ERR_CRTL;
		cpriv->error_frame.data[1] |= CAN_ERR_CRTL_RX_WARNING;
	}
	if (cpriv->status.trec & MCP25XXFD_CAN_TREC_TXBP) {
		cpriv->bus.new_state = CAN_STATE_ERROR_PASSIVE;
		cpriv->error_frame.id |= CAN_ERR_CRTL;
		cpriv->error_frame.data[1] |= CAN_ERR_CRTL_TX_PASSIVE;
	}
	if (cpriv->status.trec & MCP25XXFD_CAN_TREC_RXBP) {
		cpriv->bus.new_state = CAN_STATE_ERROR_PASSIVE;
		cpriv->error_frame.id |= CAN_ERR_CRTL;
		cpriv->error_frame.data[1] |= CAN_ERR_CRTL_RX_PASSIVE;
	}
	if (cpriv->status.trec & MCP25XXFD_CAN_TREC_TXBO) {
		cpriv->bus.new_state = CAN_STATE_BUS_OFF;
		cpriv->error_frame.id |= CAN_ERR_BUSOFF;
	}

	return 0;
}

static int mcp25xxfd_can_int_error_handling(struct mcp25xxfd_can_priv *cpriv)
{
	/* based on the last state state check the new state */
	switch (cpriv->can.state) {
	case CAN_STATE_ERROR_ACTIVE:
		if (cpriv->bus.new_state >= CAN_STATE_ERROR_WARNING &&
		    cpriv->bus.new_state <= CAN_STATE_BUS_OFF)
			cpriv->can.can_stats.error_warning++;
		/* fallthrough */
	case CAN_STATE_ERROR_WARNING:
		if (cpriv->bus.new_state >= CAN_STATE_ERROR_PASSIVE &&
		    cpriv->bus.new_state <= CAN_STATE_BUS_OFF)
			cpriv->can.can_stats.error_passive++;
		break;
	default:
		break;
	}
	cpriv->can.state = cpriv->bus.new_state;

	/* and send error packet */
	if (cpriv->error_frame.id)
		mcp25xxfd_can_int_send_error_skb(cpriv);

	/* handle BUS OFF */
	if (cpriv->can.state == CAN_STATE_BUS_OFF) {
		if (cpriv->can.restart_ms == 0) {
			cpriv->can.can_stats.bus_off++;
			can_bus_off(cpriv->can.dev);
		}
	} else {
		/* restart the tx queue if needed */
		mcp25xxfd_can_tx_queue_restart(cpriv);
	}

	return 0;
}

static int mcp25xxfd_can_int_handle_status(struct mcp25xxfd_can_priv *cpriv)
{
	int ret;

	/* clear all the interrupts asap - we have them on file allready */
	ret = mcp25xxfd_can_int_clear_int_flags(cpriv);
	if (ret)
		return ret;

	/* set up new state and error frame for this loop */
	cpriv->bus.new_state = cpriv->bus.state;
	memset(&cpriv->error_frame, 0, sizeof(cpriv->error_frame));

	/* setup the process queue by clearing the counter */
	cpriv->fifos.submit_queue_count = 0;

	/* handle interrupts */

	/* system error interrupt needs to get handled first
	 * to get us out of restricted mode
	 */
	ret = mcp25xxfd_can_int_handle_serrif(cpriv);
	if (ret)
		return ret;

	/* mode change interrupt */
	ret = mcp25xxfd_can_int_handle_modif(cpriv);
	if (ret)
		return ret;

	/* handle the rx */
	ret = mcp25xxfd_can_rx_handle_int_rxif(cpriv);
	if (ret)
		return ret;
	/* handle aborted TX FIFOs */
	ret = mcp25xxfd_can_tx_handle_int_txatif(cpriv);
	if (ret)
		return ret;

	/* handle the TEF */
	ret = mcp25xxfd_can_tx_handle_int_tefif(cpriv);
	if (ret)
		return ret;

	/* handle error interrupt flags */
	ret = mcp25xxfd_can_rx_handle_int_rxovif(cpriv);
	if (ret)
		return ret;

	/* sram ECC error interrupt */
	ret = mcp25xxfd_can_int_handle_eccif(cpriv);
	if (ret)
		return ret;

	/* message format interrupt */
	ret = mcp25xxfd_can_int_handle_ivmif(cpriv);
	if (ret)
		return ret;

	/* handle bus errors in more detail */
	ret = mcp25xxfd_can_int_handle_cerrif(cpriv);
	if (ret)
		return ret;

	/* error counter handling */
	ret = mcp25xxfd_can_int_error_counters(cpriv);
	if (ret)
		return ret;

	/* error counter handling */
	ret = mcp25xxfd_can_int_error_handling(cpriv);
	if (ret)
		return ret;

	/* and submit can frames to network stack */
	ret = mcp25xxfd_can_int_submit_frames(cpriv);

	return ret;
}

irqreturn_t mcp25xxfd_can_int(int irq, void *dev_id)
{
	struct mcp25xxfd_can_priv *cpriv = dev_id;
	int loops, ret;

	/* count interrupt calls */
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, irq_calls);

	/* loop forever unless we need to exit */
	for (loops = 0; true; loops++) {
		/* count irq loops */
		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, irq_loops);

		/* read interrupt status flags in bulk */
		ret = mcp25xxfd_cmd_read_regs(cpriv->priv->spi,
					      MCP25XXFD_CAN_INT,
					      &cpriv->status.intf,
					      sizeof(cpriv->status));
		if (ret)
			return ret;

		/* only act if the IE mask configured has active IF bits
		 * otherwise the Interrupt line should be deasserted already
		 * so we can exit the loop
		 */
		if (((cpriv->status.intf >> MCP25XXFD_CAN_INT_IE_SHIFT) &
		       cpriv->status.intf) == 0)
			break;

		/* handle the status */
		ret = mcp25xxfd_can_int_handle_status(cpriv);
		if (ret)
			return ret;

		/* allow voluntarily rescheduling every so often to avoid
		 * long CS lows at the end of a transfer on low power CPUs
		 * avoiding SERR happening
		 */
		if (loops % reschedule_int_thread_after == 0) {
			MCP25XXFD_DEBUGFS_STATS_INCR(cpriv,
						     irq_thread_rescheduled);
			cond_resched();
		}
	}

	return IRQ_HANDLED;
}

int mcp25xxfd_can_int_clear(struct mcp25xxfd_priv *priv)
{
	return mcp25xxfd_cmd_write_mask(priv->spi, MCP25XXFD_CAN_INT, 0,
					MCP25XXFD_CAN_INT_IF_MASK);
}

int mcp25xxfd_can_int_enable(struct mcp25xxfd_priv *priv, bool enable)
{
	struct mcp25xxfd_can_priv *cpriv = priv->cpriv;
	const u32 mask = MCP25XXFD_CAN_INT_TEFIE |
		MCP25XXFD_CAN_INT_RXIE |
		MCP25XXFD_CAN_INT_MODIE |
		MCP25XXFD_CAN_INT_SERRIE |
		MCP25XXFD_CAN_INT_IVMIE |
		MCP25XXFD_CAN_INT_CERRIE |
		MCP25XXFD_CAN_INT_RXOVIE |
		MCP25XXFD_CAN_INT_ECCIE;
	u32 value = cpriv ? cpriv->status.intf : 0;
	int ret;

	/* apply mask and */
	value &= ~(MCP25XXFD_CAN_INT_IE_MASK);
	if (enable)
		value |= mask;

	/* and write to int register */
	ret = mcp25xxfd_cmd_write_mask(priv->spi, MCP25XXFD_CAN_INT,
				       value, mask);
	if (ret)
		return ret;
	if (!cpriv)
		return 0;

	cpriv->status.intf = value;

	/* enable/disable interrupt handler */
	if (cpriv->irq.allocated) {
		if (enable && !cpriv->irq.enabled)
			enable_irq(cpriv->priv->spi->irq);
		if (!enable && cpriv->irq.enabled)
			disable_irq(cpriv->priv->spi->irq);
		cpriv->irq.enabled = enable;
	} else {
		cpriv->irq.enabled = false;
	}

	return 0;
}
