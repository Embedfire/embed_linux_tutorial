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
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_can.h"
#include "mcp25xxfd_can_debugfs.h"
#include "mcp25xxfd_can_id.h"
#include "mcp25xxfd_can_priv.h"
#include "mcp25xxfd_can_rx.h"

/* module parameters */
unsigned int rx_prefetch_bytes = -1;
module_param(rx_prefetch_bytes, uint, 0664);
MODULE_PARM_DESC(rx_prefetch_bytes,
		 "number of bytes to blindly prefetch when reading a rx-fifo");

static struct sk_buff *
mcp25xxfd_can_rx_submit_normal_frame(struct mcp25xxfd_can_priv *cpriv,
				     u32 id, u32 dlc, u8 **data)
{
	struct can_frame *frame;
	struct sk_buff *skb;

	/* allocate frame */
	skb = alloc_can_skb(cpriv->can.dev, &frame);
	if (!skb)
		return NULL;

	/* set id, dlc and flags */
	frame->can_id = id;
	frame->can_dlc = dlc;

	/* and set the pointer to data */
	*data = frame->data;

	return skb;
}

/* it is almost identical except for the type of the frame... */
static struct sk_buff *
mcp25xxfd_can_rx_submit_fd_frame(struct mcp25xxfd_can_priv *cpriv,
				 u32 id, u32 flags, u32 len, u8 **data)
{
	struct canfd_frame *frame;
	struct sk_buff *skb;

	/* allocate frame */
	skb = alloc_canfd_skb(cpriv->can.dev, &frame);
	if (!skb)
		return NULL;

	/* set id, dlc and flags */
	frame->can_id = id;
	frame->len = len;
	frame->flags |= flags;

	/* and set the pointer to data */
	*data = frame->data;

	return skb;
}

int mcp25xxfd_can_rx_submit_frame(struct mcp25xxfd_can_priv *cpriv, int fifo)
{
	struct net_device *net = cpriv->can.dev;
	int addr = cpriv->fifos.info[fifo].offset;
	struct mcp25xxfd_can_obj_rx *rx =
		(struct mcp25xxfd_can_obj_rx *)(cpriv->sram + addr);
	u8 *data = NULL;
	struct sk_buff *skb;
	u32 id, dlc, len, flags;

	/* compute the can_id */
	mcp25xxfd_can_id_from_mcp25xxfd(rx->id, rx->flags, &id);

	/* and dlc */
	dlc = (rx->flags & MCP25XXFD_CAN_OBJ_FLAGS_DLC_MASK) >>
		MCP25XXFD_CAN_OBJ_FLAGS_DLC_SHIFT;
	len = can_dlc2len(dlc);

	/* update stats */
	net->stats.rx_packets++;
	net->stats.rx_bytes += len;
	cpriv->fifos.rx.dlc_usage[dlc]++;
	if (rx->flags & MCP25XXFD_CAN_OBJ_FLAGS_FDF)
		MCP25XXFD_DEBUGFS_INCR(cpriv->fifos.rx.fd_count);

	/* add to rx_history */
	cpriv->rx_history.dlc[cpriv->rx_history.index] = dlc;
	cpriv->rx_history.brs[cpriv->rx_history.index] =
		(rx->flags & MCP25XXFD_CAN_OBJ_FLAGS_BRS) ? CANFD_BRS : 0;
	cpriv->rx_history.index++;
	if (cpriv->rx_history.index >= MCP25XXFD_CAN_RX_DLC_HISTORY_SIZE)
		cpriv->rx_history.index = 0;

	/* allocate the skb buffer */
	if (rx->flags & MCP25XXFD_CAN_OBJ_FLAGS_FDF) {
		flags = 0;
		flags |= (rx->flags & MCP25XXFD_CAN_OBJ_FLAGS_BRS) ?
			CANFD_BRS : 0;
		flags |= (rx->flags & MCP25XXFD_CAN_OBJ_FLAGS_ESI) ?
			CANFD_ESI : 0;
		skb = mcp25xxfd_can_rx_submit_fd_frame(cpriv, id, flags,
						       len, &data);
	} else {
		skb = mcp25xxfd_can_rx_submit_normal_frame(cpriv, id,
							   len, &data);
	}
	if (!skb) {
		netdev_err(net, "cannot allocate RX skb\n");
		net->stats.rx_dropped++;
		return -ENOMEM;
	}

	/* copy the payload data */
	memcpy(data, rx->data, len);

	/* and submit the frame */
	netif_rx_ni(skb);

	return 0;
}

static int mcp25xxfd_can_rx_read_frame(struct mcp25xxfd_can_priv *cpriv,
				       int fifo, int prefetch_bytes, bool read)
{
	struct spi_device *spi = cpriv->priv->spi;
	struct net_device *net = cpriv->can.dev;
	int addr = cpriv->fifos.info[fifo].offset;
	struct mcp25xxfd_can_obj_rx *rx =
		(struct mcp25xxfd_can_obj_rx *)(cpriv->sram + addr);
	int dlc;
	int len, ret;

	/* we read the header plus prefetch_bytes */
	if (read) {
		cpriv->stats.rx_single_reads++;
		ret = mcp25xxfd_cmd_readn(spi, MCP25XXFD_SRAM_ADDR(addr),
					  rx, sizeof(*rx) + prefetch_bytes);
		if (ret)
			return ret;
	}

	/* transpose the headers to CPU format*/
	rx->id = le32_to_cpu(rx->id);
	rx->flags = le32_to_cpu(rx->flags);
	rx->ts = le32_to_cpu(rx->ts);

	/* compute len */
	dlc = (rx->flags & MCP25XXFD_CAN_OBJ_FLAGS_DLC_MASK) >>
		MCP25XXFD_CAN_OBJ_FLAGS_DLC_SHIFT;
	len = can_dlc2len(min_t(int, dlc, (net->mtu == CANFD_MTU) ? 15 : 8));

	/* read the remaining data for canfd frames */
	if (read && len > prefetch_bytes) {
		/* update stats */
		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv,
					     rx_reads_prefetched_too_few);
		MCP25XXFD_DEBUGFS_STATS_ADD(cpriv,
					    rx_reads_prefetched_too_few_bytes,
					    len - prefetch_bytes);
		/* here the extra portion reading data after prefetch */
		ret = mcp25xxfd_cmd_readn(spi,
					  MCP25XXFD_SRAM_ADDR(addr) +
					  sizeof(*rx) + prefetch_bytes,
					  &rx->data[prefetch_bytes],
					  len - prefetch_bytes);
		if (ret)
			return ret;
	}

	/* update stats */
	cpriv->stats.rx_reads++;
	if (len < prefetch_bytes) {
		MCP25XXFD_DEBUGFS_STATS_INCR(cpriv,
					     rx_reads_prefetched_too_many);
		MCP25XXFD_DEBUGFS_STATS_ADD(cpriv,
					    rx_reads_prefetched_too_many,
					    prefetch_bytes - len);
	}

	/* clear the rest of the buffer - just to be safe */
	memset(rx->data + len, 0, ((net->mtu == CANFD_MTU) ? 64 : 8) - len);

	/* increment the statistics counter */
	MCP25XXFD_DEBUGFS_INCR(cpriv->fifos.info[fifo].use_count);

	/* add the fifo to the process queues */
	mcp25xxfd_can_queue_frame(cpriv, fifo, rx->ts, true);

	/* and clear the interrupt flag for that fifo */
	return mcp25xxfd_cmd_write_mask(spi, MCP25XXFD_CAN_FIFOCON(fifo),
					MCP25XXFD_CAN_FIFOCON_FRESET,
					MCP25XXFD_CAN_FIFOCON_FRESET);
}

static int mcp25xxfd_can_read_rx_frame_bulk(struct mcp25xxfd_can_priv *cpriv,
					    int fstart,
					    int fend)
{
	struct net_device *net = cpriv->can.dev;
	int count = abs(fend - fstart) + 1;
	int flowest = min_t(int, fstart, fend);
	int addr = cpriv->fifos.info[flowest].offset;
	struct mcp25xxfd_can_obj_rx *rx =
		(struct mcp25xxfd_can_obj_rx *)(cpriv->sram + addr);
	int len = (sizeof(*rx) + ((net->mtu == CANFD_MTU) ? 64 : 8)) * count;
	int fifo, i, ret;

	/* update stats */
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, rx_bulk_reads);
	i = min_t(int, MCP25XXFD_CAN_RX_BULK_READ_BINS - 1, count - 1);
	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, rx_bulk_read_sizes[i]);

	/* we read the header plus read_min data bytes */
	ret = mcp25xxfd_cmd_readn(cpriv->priv->spi, MCP25XXFD_SRAM_ADDR(addr),
				  rx, len);
	if (ret)
		return ret;

	/* now process all of them - no need to read... */
	for (fifo = fstart; count > 0; fifo ++, count--) {
		ret = mcp25xxfd_can_rx_read_frame(cpriv, fifo, 8, false);
		if (ret)
			return ret;
	}

	return 0;
}

/* predict dlc size based on historic behaviour */
static int mcp25xxfd_can_rx_predict_prefetch(struct mcp25xxfd_can_priv *cpriv)
{
	int dlc, i, top;
	u8 histo[16];

	/* if we have a prfecth set then use that one */
	if (rx_prefetch_bytes != -1)
		return min_t(int, rx_prefetch_bytes,
			     (cpriv->can.dev->mtu == CANFD_MTU) ? 64 : 8);

	/* memset */
	memset(histo, 0, sizeof(histo));

	/* for all others compute the histogram */
	for (i = 0; i < MCP25XXFD_CAN_RX_DLC_HISTORY_SIZE; i++)
		histo[cpriv->rx_history.dlc[i]]++;

	/* and now find the highest fit */
	for (i = (cpriv->can.dev->mtu == CANFD_MTU) ? 15 : 8, dlc = 8, top = 0;
	      i >= 0; i--) {
		if (top < histo[i]) {
			top = histo[i];
			dlc = i;
		}
	}

	/* compute length from dlc */
	cpriv->rx_history.predicted_len = can_dlc2len(dlc);

	/* return the predicted length */
	return cpriv->rx_history.predicted_len;
}

/* at least in can2.0 mode we can read multiple RX-fifos in one go
 * in case they are ajactent to each other and thus we can reduce
 * the number of spi messages produced and this improves spi-bus
 * usage efficiency.
 * In canFD mode this may also be possible, but would need some
 * statistics to decide if it is worth reading a full 64 bytes
 * in one go.
 * But those statistics can get used to predict how many bytes
 * to read together with the can header (which is fixed to 8 at
 * this very moment.
 *
 * notes on the rational here:
 * * Reading just the CAN header info takes:
 *   * bytes read
 *     *  2 bytes command+address
 *     * 12 bytes data (id, flags, timestamp)
 *   * so that is at the very least 112 SCK (= 14 byte * 8 SCK/1 byte)
 *     - on a Raspberry pi 3 for such short requests actually
 *       126 SCK (=14 byte * 9 SCK/1 byte)
 *   * some SPI framework overhead which is observed to be 5-10 us
 *     on a raspberry pi 3 (time between SCK and stop SCK start)
 *   * with an effective 17.85 MHz SPI clock on a RPI it takes in total:
 *     it takes 12us = 6us + 6us
 * * now reading 8 bytes of CAN data (can2.0) takes:
 *   * bytes read
 *     *  2 bytes command+address
 *     *  8 bytes data
 *   * so that is at the very least 80 SCK (= 10 byte * 8 SCK/1 byte)
 *     - on a Raspberry pi 3 for such short requests actually
 *       90 SCK (= 10 byte * 9 SCK/1 byte)
 *   * some SPI framework overhead which is observed to be 5-10 us
 *     on a raspberry pi 3 (time between SCK and stop SCK start)
 *   * with an effective 17.85 MHz SPI clock on a RPI it takes in total:
 *     it takes 11us = 5.0us + 6us
 * * now reading CAN header plus 8 bytes of CAN data (can2.0) takes:
 *   * bytes read
 *     *  2 bytes command+address
 *     * 20 bytes data
 *   * so that is at the very least 176 SCK (= 22 byte * 8 SCK/1 byte)
 *     - on a Raspberry pi 3 for such short requests actually
 *       198 SCK (= 22 byte * 9 SCK/1 byte)
 *   * some SPI framework overhead which is observed to be 5-10 us
 *     on a raspberry pi 3 (time between SCK and stop SCK start)
 *   * with an effective 17.85 MHz SPI clock on a RPI it takes in total:
 *     it takes 17.1us = 11.1us + 6us
 *   * this is faster than the 2 individual SPI transfers for header
 *     and data which is in total 23us
 *     * this is even true for the case where we only have a single
 *       data byte (DLC=1) - the time here is 19.5us on a RPI3
 *     * the only time where we are less efficient is for the DLC=0 case.
 *       but the assumption here is that this is a rare case
 * To put it into perspective here the full table for a RPI3:
 * LE 2m  pr0 pr1 pr2 pr3 pr4 pr5  pr6  pr7  pr8 pr12 pr16 pr20 pr24 pr32 pr48
 *                                                                         pr64
 *  0  7.1 7.1
 *  1 14.6    7.6 8.1 8.6 9.1 9.6 10.1 10.6 11.1 13.1
 *  2 15.1        8.1 8.6 9.1 9.6 10.1 10.6 11.1 13.1
 *  3 15.6            8.6 9.1 9.6 10.1 10.6 11.1 13.1 15.1
 *  4 16.1                9.1 9.6 10.1 10.6 11.1 13.1 15.1
 *  5 16.6                    9.6 10.1 10.6 11.1 13.1 15.1
 *  6 17.1                        10.1 10.6 11.1 13.1 15.1
 *  7 17.6                             10.6 11.1 13.1 15.1 17.1
 *  8 18.1                                  11.1 13.1 15.1 17.1
 * 12 20.1                                       13.1 15.1 17.1 19.2
 * 16 22.1                                            15.1 17.1 19.2
 * 20 24.1                                                 17.1 19.2 23.2
 * 24 26.2                                                      19.2 23.2
 * 32 30.2                                                           23.2
 * 48 38.3                                                                31.3
 * 64 46.3                                                                 39.3
 * (Parameters: SPI Clock=17.8MHz, SCK/byte=9, overhead=6us)
 * Legend:
 *   LE = length,
 *   2m    = 2 SPI messages (header+data - except for LEN=0, only header)
 *  prX/pX = prefecth length times (only shown when < 2m and Len >= Prefetch)
 *
 * The diagonal schows the "optimal" time when the size of the Can frame would
 * be known ahead of time - i.e if it would be possible to define RX reception
 * filters based on can DLC values
 *
 * So for any Can frame except for LEN=0 the prefetch data solution is
 * better for prefetch of data=12 for CanFD.
 *
 * Here another table showing the optimal prefetch limits for SPI speeds
 * vs overhead_us at 8 or 9 SCLK/byte
 *
 * MHZ  2us@8   2us@9   4us@8   4us@9   6us@8   6us@9   8us@8   8us@9
 * 10.0 8b***   8b***   8b      8b*     12b**   8b*     12b     12b*
 * 12.5 8b**    8b***   12b***  8b      12b     12b*    16b*    16b**
 * 15.0 8b**    8b**    12b**   12b***  16b**   12b     20b**   16b
 * 17.5 8b*     8b*     12b*    12b**   16b     16b**   20b     20b**
 * 20.0 8b      8b*     16b***  12b*    20b**   16b     24b*    20b
 * (a * signifies not a full match, but for any length > count(*))
 *
 * So 8 bytes prefetch seems to be a very good tradeoff for can frame
 * except for DLC/LEN=0 frames.
 * The question here is mainly: how many frames do we have with DLC=0
 * vs all others.
 *
 * With some statistics of recent CAN frames this may be set dynamically
 * in the future.
 *
 * For this to work efficiently we would also need an estimate on
 * the SPI framework overhead, which is a function of the spi-bus-driver
 * implementation details, CPU type and speed as well as system load.
 * Also the effective SPI-clock speed is needed as well as the
 * number of spi clock cycles it takes for a single byte to get transferred
 * The bcm283x SOC for example pauses the SPI clock one cycle after
 * every byte it sends unless the data is fed to the controller by DMA.
 * (but for short transfers DMA mapping is very expensive and not worth
 * the effort. PIO and - in some situations - polling is used instead to
 * reduce the number of interrupts and the need for thread scheduling as
 * much as possible)
 *
 * This also means that for can2.0 only configured interfaces
 * reading multiple rx fifos is a realistic option of optimization
 */

static int mcp25xxfd_can_rx_read_single_frames(struct mcp25xxfd_can_priv *cpriv,
					       int prefetch)
{
	int i, f, ret;

	/* loop all frames */
	for (i = 0, f = cpriv->fifos.rx.start; i < cpriv->fifos.rx.count;
	     i++, f++) {
		if (cpriv->status.rxif & BIT(f)) {
			/* read the frame */
			ret = mcp25xxfd_can_rx_read_frame(cpriv, f,
							  prefetch, true);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int mcp25xxfd_can_rx_read_bulk_frames(struct mcp25xxfd_can_priv *cpriv)
{
	int i, start, end;
	int ret;

	/* iterate over fifos trying to find fifos next to each other */
	for (i = 0, start = cpriv->fifos.rx.start, end = start;
	     i < cpriv->fifos.rx.count; i++, end++, start = end) {
		/* if bit is not set then continue */
		if (!(cpriv->status.rxif & BIT(start)))
			continue;
		/* find the last fifo with a bit set in sequence */
		for (end = start; cpriv->status.rxif & BIT(end + 1); end++)
			;
		/* and now read those fifos in bulk */
		ret = mcp25xxfd_can_read_rx_frame_bulk(cpriv, start, end);
		if (ret)
			return ret;
	}

	return 0;
}

static int mcp25xxfd_can_rx_read_fd_frames(struct mcp25xxfd_can_priv *cpriv)
{
	int i, count_dlc15, count_brs, prefetch;

	/* get a prediction on prefetch */
	prefetch = mcp25xxfd_can_rx_predict_prefetch(cpriv);

	/* if the prefetch is < 64 then just read single */
	if (prefetch < 64)
		return mcp25xxfd_can_rx_read_single_frames(cpriv, prefetch);

	/* check if we have mostly brs frames of those DLC=15 frames */
	for (i = 0, count_brs = 0, count_dlc15 = 0;
	     i < MCP25XXFD_CAN_RX_DLC_HISTORY_SIZE; i++)
		if (cpriv->rx_history.dlc[i] == 15) {
			count_dlc15++;
			if (cpriv->rx_history.brs[i])
				count_brs++;
		}

	/* if we have at least 33% brs frames then run bulk */
	if (count_brs * 3 >= count_dlc15 )
		return mcp25xxfd_can_rx_read_bulk_frames(cpriv);
	else
		return mcp25xxfd_can_rx_read_single_frames(cpriv, prefetch);
}

static int mcp25xxfd_can_rx_read_frames(struct mcp25xxfd_can_priv *cpriv)
{
	if (cpriv->can.dev->mtu == CANFD_MTU)
		return mcp25xxfd_can_rx_read_fd_frames(cpriv);
	else
		return mcp25xxfd_can_rx_read_bulk_frames(cpriv);
}

int mcp25xxfd_can_rx_handle_int_rxif(struct mcp25xxfd_can_priv *cpriv)
{
	if (!cpriv->status.rxif)
		return 0;

	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_rx_count);

	/* read all the fifos */
	return mcp25xxfd_can_rx_read_frames(cpriv);
}

int mcp25xxfd_can_rx_handle_int_rxovif(struct mcp25xxfd_can_priv *cpriv)
{
	u32 mask = MCP25XXFD_CAN_FIFOSTA_RXOVIF;
	int ret, i, reg;

	if (!cpriv->status.rxovif)
		return 0;

	MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, int_rxov_count);

	/* clear all fifos that have an overflow bit set */
	for (i = 0; i < 32; i++) {
		if (cpriv->status.rxovif & BIT(i)) {
			/* clear fifo status */
			reg = MCP25XXFD_CAN_FIFOSTA(i);
			ret = mcp25xxfd_cmd_write_mask(cpriv->priv->spi,
						       reg, 0, mask);
			if (ret)
				return ret;

			/* update statistics */
			cpriv->can.dev->stats.rx_over_errors++;
			cpriv->can.dev->stats.rx_errors++;

			/* and prepare ERROR FRAME */
			cpriv->error_frame.id |= CAN_ERR_CRTL;
			cpriv->error_frame.data[1] |=
				CAN_ERR_CRTL_RX_OVERFLOW;
		}
	}

	return 0;
}
