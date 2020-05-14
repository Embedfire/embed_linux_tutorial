// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

/* controller details
 *
 *  It has 32 FIFOs (of up to 32 CAN-frames).
 *
 * There are 4 Fifo types which can get configured:
 * * TEF - Transmission Event Fifo - which consumes FIFO 0
 *   even if it is not configured
 * * Tansmission Queue - for up to 32 Frames.
 *   this queue reorders CAN frames to get transmitted following the
 *   typical CAN dominant/recessive rules on the can bus itself.
 *   This FIFO is optional.
 * * TX FIFO: generic TX fifos that can contain arbitrary data
 *   and which come with a configurable priority for transmission
 *   It is also possible to have the Controller automatically trigger
 *   a transfer when a Filter Rule for a RTR frame matches.
 *   Each of these fifos in principle can get configured for distinct
 *   dlc sizes (8 thru 64 bytes)
 * * RX FIFO: generic RX fifo which is filled via filter-rules.
 *   Each of these fifos in principle can get configured for distinct
 *   dlc sizes (8 thru 64 bytes)
 *   Unfortunately there is no filter rule that would allow triggering
 *   on different frame sizes, so for all practical purposes the
 *   RX fifos have to be of the same size (unless one wants to experience
 *   lost data).
 * When a Can Frame is transmitted from the TX Queue or an individual
 * TX FIFO then a small TEF Frame can get added to the TEF FIFO queue
 * to log the Transmission of the frame - this includes ID, Flags
 * (including a custom identifier/index) and the timestamp (see below).
 *
 * The controller provides an optional free running counter with a divider
 * for timestamping of RX frames as well as for TEF entries.
 */

#include <linux/can/core.h>
#include <linux/can/dev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_base.h"
#include "mcp25xxfd_can_debugfs.h"
#include "mcp25xxfd_can_fifo.h"
#include "mcp25xxfd_can_int.h"
#include "mcp25xxfd_can_priv.h"
#include "mcp25xxfd_can_tx.h"
#include "mcp25xxfd_clock.h"
#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_int.h"
#include "mcp25xxfd_priv.h"
#include "mcp25xxfd_regs.h"

#include <uapi/linux/can/netlink.h>

/* module parameters */
unsigned int bw_sharing_log2bits;
module_param(bw_sharing_log2bits, uint, 0664);
MODULE_PARM_DESC(bw_sharing_log2bits,
		 "Delay between 2 transmissions in number of arbitration bit times\n");
bool enable_edge_filter;
module_param(enable_edge_filter, bool, 0664);
MODULE_PARM_DESC(enable_edge_filter,
		 "Enable ISO11898-1:2015 edge_filtering");
unsigned int tdc_mode = 2;
module_param(tdc_mode, uint, 0664);
MODULE_PARM_DESC(tdc_mode,
		 "Transmitter Delay Mode - 0 = disabled, 1 = fixed, 2 = auto\n");
unsigned int tdc_value;
module_param(tdc_value, uint, 0664);
MODULE_PARM_DESC(tdc_value,
		 "Transmission Delay Value - range: [0:63] SCLK");
int tdc_offset = 64; /* outside of range to use computed values */
module_param(tdc_offset, int, 0664);
MODULE_PARM_DESC(tdc_offset,
		 "Transmission Delay offset - range: [-64:63] SCLK");

/* everything related to bit timing */
static
const struct can_bittiming_const mcp25xxfd_can_nominal_bittiming_const = {
	.name           = DEVICE_NAME,
	.tseg1_min      = 2,
	.tseg1_max      = BIT(MCP25XXFD_CAN_NBTCFG_TSEG1_BITS),
	.tseg2_min      = 1,
	.tseg2_max      = BIT(MCP25XXFD_CAN_NBTCFG_TSEG2_BITS),
	.sjw_max        = BIT(MCP25XXFD_CAN_NBTCFG_SJW_BITS),
	.brp_min        = 1,
	.brp_max        = BIT(MCP25XXFD_CAN_NBTCFG_BRP_BITS),
	.brp_inc        = 1,
};

static
const struct can_bittiming_const mcp25xxfd_can_data_bittiming_const = {
	.name           = DEVICE_NAME,
	.tseg1_min      = 1,
	.tseg1_max      = BIT(MCP25XXFD_CAN_DBTCFG_TSEG1_BITS),
	.tseg2_min      = 1,
	.tseg2_max      = BIT(MCP25XXFD_CAN_DBTCFG_TSEG2_BITS),
	.sjw_max        = BIT(MCP25XXFD_CAN_DBTCFG_SJW_BITS),
	.brp_min        = 1,
	.brp_max        = BIT(MCP25XXFD_CAN_DBTCFG_BRP_BITS),
	.brp_inc        = 1,
};

static int mcp25xxfd_can_do_set_nominal_bittiming(struct net_device *net)
{
	struct mcp25xxfd_can_priv *cpriv = netdev_priv(net);
	struct can_bittiming *bt = &cpriv->can.bittiming;

	int sjw = bt->sjw;
	int pseg2 = bt->phase_seg2;
	int pseg1 = bt->phase_seg1;
	int propseg = bt->prop_seg;
	int brp = bt->brp;

	int tseg1 = propseg + pseg1;
	int tseg2 = pseg2;

	/* calculate nominal bit timing */
	cpriv->regs.nbtcfg = ((sjw - 1) << MCP25XXFD_CAN_NBTCFG_SJW_SHIFT) |
		((tseg2 - 1) << MCP25XXFD_CAN_NBTCFG_TSEG2_SHIFT) |
		((tseg1 - 1) << MCP25XXFD_CAN_NBTCFG_TSEG1_SHIFT) |
		((brp - 1) << MCP25XXFD_CAN_NBTCFG_BRP_SHIFT);

	return mcp25xxfd_cmd_write(cpriv->priv->spi, MCP25XXFD_CAN_NBTCFG,
				   cpriv->regs.nbtcfg);
}

static int mcp25xxfd_can_do_set_data_bittiming(struct net_device *net)
{
	struct mcp25xxfd_can_priv *cpriv = netdev_priv(net);
	struct mcp25xxfd_priv *priv = cpriv->priv;
	struct can_bittiming *bt = &cpriv->can.data_bittiming;
	struct spi_device *spi = priv->spi;

	int sjw = bt->sjw;
	int pseg2 = bt->phase_seg2;
	int pseg1 = bt->phase_seg1;
	int propseg = bt->prop_seg;
	int brp = bt->brp;

	int tseg1 = propseg + pseg1;
	int tseg2 = pseg2;

	int tdco;
	int ret;

	/* set up Transmitter delay compensation */
	cpriv->regs.tdc = 0;
	/* configure TDC mode */
	if (tdc_mode < 4)
		cpriv->regs.tdc = tdc_mode << MCP25XXFD_CAN_TDC_TDCMOD_SHIFT;
	else
		cpriv->regs.tdc = MCP25XXFD_CAN_TDC_TDCMOD_AUTO <<
			MCP25XXFD_CAN_TDC_TDCMOD_SHIFT;

	/* configure TDC offsets */
	if ((tdc_offset >= -64) && tdc_offset < 64)
		tdco = tdc_offset;
	else
		tdco = clamp_t(int, bt->brp * tseg1, -64, 63);
	cpriv->regs.tdc |= (tdco << MCP25XXFD_CAN_TDC_TDCO_SHIFT) &
		MCP25XXFD_CAN_TDC_TDCO_MASK;

	/* configure TDC value */
	if (tdc_value < 64)
		cpriv->regs.tdc |= tdc_value << MCP25XXFD_CAN_TDC_TDCV_SHIFT;

	/* enable edge filtering */
	if (enable_edge_filter)
		cpriv->regs.tdc |= MCP25XXFD_CAN_TDC_EDGFLTEN;

	/* set TDC */
	ret = mcp25xxfd_cmd_write(spi, MCP25XXFD_CAN_TDC, cpriv->regs.tdc);
	if (ret)
		return ret;

	/* calculate data bit timing */
	cpriv->regs.dbtcfg = ((sjw - 1) << MCP25XXFD_CAN_DBTCFG_SJW_SHIFT) |
		((tseg2 - 1) << MCP25XXFD_CAN_DBTCFG_TSEG2_SHIFT) |
		((tseg1 - 1) << MCP25XXFD_CAN_DBTCFG_TSEG1_SHIFT) |
		((brp - 1) << MCP25XXFD_CAN_DBTCFG_BRP_SHIFT);

	return mcp25xxfd_cmd_write(spi, MCP25XXFD_CAN_DBTCFG,
				   cpriv->regs.dbtcfg);
}

int mcp25xxfd_can_get_mode(struct mcp25xxfd_priv *priv, u32 *reg)
{
	int ret;

	ret = mcp25xxfd_cmd_read(priv->spi, MCP25XXFD_CAN_CON, reg);
	if (ret)
		return ret;

	return (*reg & MCP25XXFD_CAN_CON_OPMOD_MASK) >>
		MCP25XXFD_CAN_CON_OPMOD_SHIFT;
}

int mcp25xxfd_can_switch_mode_no_wait(struct mcp25xxfd_priv *priv,
				      u32 *reg, int mode)
{
	u32 dummy;
	int ret;

	/* get the current mode/register - if reg is NULL
	 * when the can controller is not setup yet
	 * typically by calling mcp25xxfd_can_sleep_mode
	 * (this only happens during initialization phase)
	 */
	if (reg) {
		if (!reg) {
			ret = mcp25xxfd_can_get_mode(priv, reg);
			if (ret < 0)
				return ret;
		}
	} else {
		/* alternatively use dummy */
		dummy = 0;
		reg = &dummy;
	}

	/* compute the effective mode in osc*/
	*reg &= ~(MCP25XXFD_CAN_CON_REQOP_MASK |
		  MCP25XXFD_CAN_CON_OPMOD_MASK);
	*reg |= (mode << MCP25XXFD_CAN_CON_REQOP_SHIFT) |
		(mode << MCP25XXFD_CAN_CON_OPMOD_SHIFT);

	/* if the opmode is sleep then the oscilator will be disabled
	 * and also not ready, so fake this change
	 */
	if (mode == MCP25XXFD_CAN_CON_MODE_SLEEP)
		mcp25xxfd_clock_fake_sleep(priv);

	/* request the mode switch */
	return mcp25xxfd_cmd_write(priv->spi, MCP25XXFD_CAN_CON, *reg);
}

int mcp25xxfd_can_switch_mode(struct mcp25xxfd_priv *priv, u32 *reg, int mode)
{
	int ret, i;

	/* trigger the mode switch itself */
	ret = mcp25xxfd_can_switch_mode_no_wait(priv, reg, mode);
	if (ret)
		return ret;

	/* if we are in now sleep mode then return immediately
	 * the controller does not respond back!
	 */
	if (mode == MCP25XXFD_CAN_CON_MODE_SLEEP)
		return 0;

	/* wait for it to stabilize/switch mode
	 * we assume 256 rounds should be enough as this is > 12ms
	 * at 1MHz Can Bus speed without any extra overhead
	 *
	 * The assumption here is that it depends on bus activity
	 * how long it takes the controller to switch modes
	 */
	for (i = 0; i < 256; i++) {
		/* get the mode */
		ret = mcp25xxfd_can_get_mode(priv, reg);
		if (ret < 0)
			return ret;
		/* check that we have reached our mode */
		if (ret == mode)
			return 0;
	}

	dev_err(&priv->spi->dev, "Failed to switch to mode %u in time\n",
		mode);
	return -ETIMEDOUT;
}

static int mcp25xxfd_can_probe_modeswitch(struct mcp25xxfd_priv *priv)
{
	u32 mode_data;
	int ret;

	/* so we should be in config mode now, so move to INT_LOOPBACK */
	ret = mcp25xxfd_can_switch_mode(priv, &mode_data,
					MCP25XXFD_CAN_CON_MODE_INT_LOOPBACK);
	if (ret) {
		dev_err(&priv->spi->dev,
			"Failed to switch into loopback mode\n");
		return ret;
	}

	/* and back into config mode */
	ret = mcp25xxfd_can_switch_mode(priv, &mode_data,
					MCP25XXFD_CAN_CON_MODE_CONFIG);
	if (ret) {
		dev_err(&priv->spi->dev,
			"Failed to switch back to config mode\n");
		return ret;
	}

	/* so we have checked basic functionality successfully */
	return 0;
}

int mcp25xxfd_can_sleep_mode(struct mcp25xxfd_priv *priv)
{
	return mcp25xxfd_can_switch_mode(priv, NULL,
					 MCP25XXFD_CAN_CON_MODE_SLEEP);
}

int mcp25xxfd_can_probe(struct mcp25xxfd_priv *priv)
{
	struct spi_device *spi = priv->spi;
	u32 mode_data;
	int mode, ret;

	/* read TXQCON - the TXEN bit should always read as 1 */
	ret = mcp25xxfd_cmd_read(spi, MCP25XXFD_CAN_TXQCON, &mode_data);
	if (ret)
		return ret;
	if ((mode_data & MCP25XXFD_CAN_TXQCON_TXEN) == 0) {
		dev_err(&spi->dev,
			"Register TXQCON does not have bit TXEN set - reads as %08x - this may be a problem with spi bus signal quality - try reducing spi-clock speed if this can get reproduced",
			mode_data);
		return -EINVAL;
	}

	/* try to get the current mode */
	mode = mcp25xxfd_can_get_mode(priv, &mode_data);
	if (mode < 0)
		return mode;

	/* we would expect to be in config mode, as a SPI-reset should
	 * have moved us into config mode.
	 * But then the documentation says that SPI-reset may only work
	 * reliably when already in config mode
	 */

	/* so if we are in config mode then everything is fine
	 * and we check that a mode switch works propperly
	 */
	if (mode == MCP25XXFD_CAN_CON_MODE_CONFIG)
		return mcp25xxfd_can_probe_modeswitch(priv);

	/* if the bitfield is 0 then there is something is wrong */
	if (!mode_data) {
		dev_err(&spi->dev,
			"got controller config register reading as 0\n");
		return -EINVAL;
	}

	/* any other mode is unexpected */
	dev_err(&spi->dev,
		"Found controller in unexpected mode %i - register reads as %08x\n",
		mode, mode_data);

	/* so try to move to config mode
	 * if this fails, then everything is lost and the controller
	 * is not identified
	 * This action MAY be destructive if a different device is connected
	 * but note that the first hurdle (oscillator) was already
	 * successful - so we should be safe...
	 */
	ret = mcp25xxfd_can_switch_mode(priv, &mode_data,
					MCP25XXFD_CAN_CON_MODE_CONFIG);
	if (ret) {
		dev_err(&priv->spi->dev,
			"Mode did not switch to config as expected - could not identify controller - register reads as %08x\n",
			mode_data);
		return -EINVAL;
	}
	/* check that modeswitch is really working */
	return mcp25xxfd_can_probe_modeswitch(priv);
}

static int mcp25xxfd_can_config(struct net_device *net)
{
	struct mcp25xxfd_can_priv *cpriv = netdev_priv(net);
	struct mcp25xxfd_priv *priv = cpriv->priv;
	struct spi_device *spi = priv->spi;
	int ret;

	/* setup value of con_register */
	cpriv->regs.con = MCP25XXFD_CAN_CON_STEF; /* enable TEF, disable TXQ */

	/* transmission bandwidth sharing bits */
	if (bw_sharing_log2bits > 12)
		bw_sharing_log2bits = 12;
	cpriv->regs.con |= bw_sharing_log2bits <<
		MCP25XXFD_CAN_CON_TXBWS_SHIFT;

	/* non iso FD mode */
	if (!(cpriv->can.ctrlmode & CAN_CTRLMODE_FD_NON_ISO))
		cpriv->regs.con |= MCP25XXFD_CAN_CON_ISOCRCEN;

	/* one shot */
	if (cpriv->can.ctrlmode & CAN_CTRLMODE_ONE_SHOT)
		cpriv->regs.con |= MCP25XXFD_CAN_CON_RTXAT;

	/* apply it now together with a mode switch */
	ret = mcp25xxfd_can_switch_mode(cpriv->priv, &cpriv->regs.con,
					MCP25XXFD_CAN_CON_MODE_CONFIG);
	if (ret)
		return 0;

	/* time stamp control register - 1ns resolution */
	cpriv->regs.tscon = 0;
	ret = mcp25xxfd_cmd_write(spi, MCP25XXFD_CAN_TBC, 0);
	if (ret)
		return ret;

	cpriv->regs.tscon = MCP25XXFD_CAN_TSCON_TBCEN |
		((cpriv->can.clock.freq / 1000000)
		 << MCP25XXFD_CAN_TSCON_TBCPRE_SHIFT);
	ret = mcp25xxfd_cmd_write(spi, MCP25XXFD_CAN_TSCON, cpriv->regs.tscon);
	if (ret)
		return ret;

	/* setup fifos */
	ret = mcp25xxfd_can_fifo_setup(cpriv);
	if (ret)
		return ret;

	/* setup can bittiming now - the do_set_bittiming methods
	 * are not used as they get callled before open
	 */
	ret = mcp25xxfd_can_do_set_nominal_bittiming(net);
	if (ret)
		return ret;
	ret = mcp25xxfd_can_do_set_data_bittiming(net);
	if (ret)
		return ret;

	return ret;
}

/* mode setting */
static int mcp25xxfd_can_do_set_mode(struct net_device *net,
				     enum can_mode mode)
{
	switch (mode) {
	case CAN_MODE_START:
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

/* binary error counters */
static int mcp25xxfd_can_get_berr_counter(const struct net_device *net,
					  struct can_berr_counter *bec)
{
	struct mcp25xxfd_can_priv *cpriv = netdev_priv(net);

	bec->txerr = (cpriv->status.trec & MCP25XXFD_CAN_TREC_TEC_MASK) >>
		MCP25XXFD_CAN_TREC_TEC_SHIFT;
	bec->rxerr = (cpriv->status.trec & MCP25XXFD_CAN_TREC_REC_MASK) >>
		MCP25XXFD_CAN_TREC_REC_SHIFT;

	return 0;
}

static int mcp25xxfd_can_open(struct net_device *net)
{
	struct mcp25xxfd_can_priv *cpriv = netdev_priv(net);
	struct spi_device *spi = cpriv->priv->spi;
	int ret;

	ret = open_candev(net);
	if (ret) {
		netdev_err(net, "unable to set initial baudrate!\n");
		return ret;
	}

	/* clear those statistics */
	memset(&cpriv->stats, 0, sizeof(cpriv->stats));

	/* request an IRQ but keep disabled for now */
	ret = request_threaded_irq(spi->irq, NULL, 
				   mcp25xxfd_can_int,
				   IRQF_ONESHOT | IRQ_TYPE_EDGE_FALLING,
				   cpriv->priv->device_name, cpriv);
	if (ret) {
		dev_err(&spi->dev, "failed to acquire irq %d - %i\n",
			spi->irq, ret);
		goto out_candev;
	}

	disable_irq(spi->irq);
	cpriv->irq.allocated = true;
	cpriv->irq.enabled = false;

	/* enable power to the transceiver */
	ret = mcp25xxfd_base_power_enable(cpriv->transceiver, 1);
	if (ret)
		goto out_irq;

	/* enable clock (so that spi works) */
	ret = mcp25xxfd_clock_start(cpriv->priv, MCP25XXFD_CLK_USER_CAN);
	if (ret)
		goto out_transceiver;

	/* configure controller for reception */
	ret = mcp25xxfd_can_config(net);
	if (ret)
		goto out_canclock;

	/* setting up state */
	cpriv->can.state = CAN_STATE_ERROR_ACTIVE;

	/* enable interrupts */
	ret = mcp25xxfd_int_enable(cpriv->priv, true);
	if (ret)
		goto out_canconfig;

	/* switch to active mode */
	ret = mcp25xxfd_can_switch_mode(cpriv->priv, &cpriv->regs.con,
					(net->mtu == CAN_MTU) ?
					MCP25XXFD_CAN_CON_MODE_CAN2_0 :
					MCP25XXFD_CAN_CON_MODE_MIXED);
	if (ret)
		goto out_int;

	/* start the tx_queue */
	mcp25xxfd_can_tx_queue_manage(cpriv,
				      MCP25XXFD_CAN_TX_QUEUE_STATE_STARTED);

	return 0;

out_int:
	mcp25xxfd_int_enable(cpriv->priv, false);
out_canconfig:
	mcp25xxfd_can_fifo_release(cpriv);
out_canclock:
	mcp25xxfd_clock_stop(cpriv->priv, MCP25XXFD_CLK_USER_CAN);
out_transceiver:
	mcp25xxfd_base_power_enable(cpriv->transceiver, 0);
out_irq:
	free_irq(spi->irq, cpriv);
	cpriv->irq.allocated = false;
	cpriv->irq.enabled = false;
out_candev:
	close_candev(net);
	return ret;
}

static void mcp25xxfd_can_shutdown(struct mcp25xxfd_can_priv *cpriv)
{
	/* switch us to CONFIG mode - this disables the controller */
	mcp25xxfd_can_switch_mode(cpriv->priv, &cpriv->regs.con,
				  MCP25XXFD_CAN_CON_MODE_CONFIG);
}

static int mcp25xxfd_can_stop(struct net_device *net)
{
	struct mcp25xxfd_can_priv *cpriv = netdev_priv(net);
	struct mcp25xxfd_priv *priv = cpriv->priv;
	struct spi_device *spi = priv->spi;

	/* stop transmit queue */
	mcp25xxfd_can_tx_queue_manage(cpriv,
				      MCP25XXFD_CAN_TX_QUEUE_STATE_STOPPED);

	/* release fifos and debugfs */
	mcp25xxfd_can_fifo_release(cpriv);

	/* shutdown the can controller */
	mcp25xxfd_can_shutdown(cpriv);

	/* disable inerrupts on controller */
	mcp25xxfd_int_enable(cpriv->priv, false);

	/* stop the clock */
	mcp25xxfd_clock_stop(cpriv->priv, MCP25XXFD_CLK_USER_CAN);

	/* and disable the transceiver */
	mcp25xxfd_base_power_enable(cpriv->transceiver, 0);

	/* disable interrupt on host */
	free_irq(spi->irq, cpriv);
	cpriv->irq.allocated = false;
	cpriv->irq.enabled = false;

	/* close the can_decice */
	close_candev(net);

	return 0;
}

static const struct net_device_ops mcp25xxfd_netdev_ops = {
	.ndo_open = mcp25xxfd_can_open,
	.ndo_stop = mcp25xxfd_can_stop,
	.ndo_start_xmit = mcp25xxfd_can_tx_start_xmit,
	.ndo_change_mtu = can_change_mtu,
};

/* probe and remove */
int mcp25xxfd_can_setup(struct mcp25xxfd_priv *priv)
{
	struct spi_device *spi = priv->spi;
	struct mcp25xxfd_can_priv *cpriv;
	struct net_device *net;
	struct regulator *transceiver;
	int ret;

	/* get transceiver power regulator*/
	transceiver = devm_regulator_get_optional(&spi->dev,
						  "xceiver");
	if (PTR_ERR(transceiver) == -EPROBE_DEFER)
		return -EPROBE_DEFER;

	/* allocate can device */
	net = alloc_candev(sizeof(*cpriv), TX_ECHO_SKB_MAX);
	if (!net)
		return -ENOMEM;

	/* and do some cross-asignments */
	cpriv = netdev_priv(net);
	cpriv->priv = priv;
	priv->cpriv = cpriv;

	/* setup network */
	SET_NETDEV_DEV(net, &spi->dev);
	net->netdev_ops = &mcp25xxfd_netdev_ops;
	net->flags |= IFF_ECHO;

	/* assign transceiver */
	cpriv->transceiver = transceiver;

	/* setup can */
	cpriv->can.clock.freq = priv->clock_freq;
	cpriv->can.bittiming_const =
		&mcp25xxfd_can_nominal_bittiming_const;
	cpriv->can.data_bittiming_const =
		&mcp25xxfd_can_data_bittiming_const;
	/* we are not setting bit-timing methods here as they get
	 * called by the framework before open so the controller is
	 * still in sleep mode, which does not help
	 * things are configured in open instead
	 */
	cpriv->can.do_set_mode =
		mcp25xxfd_can_do_set_mode;
	cpriv->can.do_get_berr_counter =
		mcp25xxfd_can_get_berr_counter;
	cpriv->can.ctrlmode_supported =
		CAN_CTRLMODE_FD |
		CAN_CTRLMODE_FD_NON_ISO |
		CAN_CTRLMODE_LOOPBACK |
		CAN_CTRLMODE_LISTENONLY |
		CAN_CTRLMODE_BERR_REPORTING |
		CAN_CTRLMODE_ONE_SHOT;

	ret = register_candev(net);
	if (ret) {
		dev_err(&spi->dev, "Failed to register can device\n");
		goto out;
	}

	mcp25xxfd_can_debugfs_setup(cpriv);

	return 0;
out:
	free_candev(net);
	priv->cpriv = NULL;

	return ret;
}

void mcp25xxfd_can_remove(struct mcp25xxfd_priv *priv)
{
	if (priv->cpriv) {
		unregister_candev(priv->cpriv->can.dev);
		free_candev(priv->cpriv->can.dev);
		priv->cpriv = NULL;
	}
}
