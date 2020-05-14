// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_priv.h"

/* SPI helper */

/* wrapper arround spi_sync, that sets speed_hz */
static int mcp25xxfd_cmd_sync_transfer(struct spi_device *spi,
				       struct spi_transfer *xfer,
				       unsigned int xfers)
{
	struct mcp25xxfd_priv *priv = spi_get_drvdata(spi);
	int i;

	for (i = 0; i < xfers; i++)
		xfer[i].speed_hz = priv->spi_use_speed_hz;

	return spi_sync_transfer(spi, xfer, xfers);
}

/* simple spi_write wrapper with speed_hz
 * WARINING: tx_buf needs to be on heap!
 */
static int mcp25xxfd_cmd_sync_write(struct spi_device *spi,
				    const void *tx_buf,
				    unsigned int tx_len)
{
	struct spi_transfer xfer;

	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf = tx_buf;
	xfer.len = tx_len;

	return mcp25xxfd_cmd_sync_transfer(spi, &xfer, 1);
}

/* alloc buffer */
static int mcp25xxfd_cmd_alloc_buf(struct spi_device *spi,
				   size_t len,
				   u8 **tx, u8 **rx)
{
	struct mcp25xxfd_priv *priv = spi_get_drvdata(spi);

	/* allocate from heap in case the size is to big
	 * or the preallocated buffer is already used (i.e locked)
	 */
	if (len > sizeof(priv->spi_tx) ||
	    !mutex_trylock(&priv->spi_rxtx_lock)) {
		/* allocate tx+rx in one allocation if rx is requested */
		*tx = kzalloc(rx ? 2 * len : len, GFP_KERNEL);
		if (!*tx)
			return -ENOMEM;
		if (rx)
			*rx = *tx + len;
	} else {
		/* use the preallocated buffers instead */
		*tx = priv->spi_tx;
		memset(priv->spi_tx, 0, sizeof(priv->spi_tx));
		if (rx) {
			*rx = priv->spi_rx;
			memset(priv->spi_rx, 0, sizeof(priv->spi_rx));
		}
	}

	return 0;
}

static void mcp25xxfd_cmd_release_buf(struct spi_device *spi, u8 *tx, u8 *rx)
{
	struct mcp25xxfd_priv *priv = spi_get_drvdata(spi);

	if (tx == priv->spi_tx)
		mutex_unlock(&priv->spi_rxtx_lock);
	else
		kfree(tx);
}

/* an optimization of spi_write_then_read that merges the transfers
 * this also makes sure that the data is ALWAYS on heap
 */
static int mcp25xxfd_cmd_write_then_read(struct spi_device *spi,
					 const void *tx_buf,
					 unsigned int tx_len,
					 void *rx_buf,
					 unsigned int rx_len)
{
	struct spi_transfer xfer[2];
	u8 *spi_tx, *spi_rx;
	int xfers;
	int ret;

	/* get pointer to buffers */
	ret = mcp25xxfd_cmd_alloc_buf(spi, tx_len + rx_len, &spi_tx, &spi_rx);
	if (ret)
		return ret;

	/* clear the xfers */
	memset(xfer, 0, sizeof(xfer));

	/* special handling for half-duplex */
	if (spi->master->flags & SPI_MASTER_HALF_DUPLEX) {
		xfers = 2;
		xfer[0].tx_buf = spi_tx;
		xfer[0].len = tx_len;
		/* the offset for rx_buf needs to get aligned */
		xfer[1].rx_buf = spi_rx + tx_len;
		xfer[1].len = rx_len;
	} else {
		xfers = 1;
		xfer[0].len = tx_len + rx_len;
		xfer[0].tx_buf = spi_tx;
		xfer[0].rx_buf = spi_rx;
	}

	/* copy data - especially to avoid buffers from stack */
	memcpy(spi_tx, tx_buf, tx_len);

	/* do the transfer */
	ret = mcp25xxfd_cmd_sync_transfer(spi, xfer, xfers);
	if (ret)
		goto out;

	/* copy result back */
	memcpy(rx_buf, xfer[0].rx_buf + tx_len, rx_len);

out:
	mcp25xxfd_cmd_release_buf(spi, spi_tx, spi_rx);

	return ret;
}

static int mcp25xxfd_cmd_write_then_write(struct spi_device *spi,
					  const void *tx_buf,
					  unsigned int tx_len,
					  const void *tx2_buf,
					  unsigned int tx2_len)
{
	struct spi_transfer xfer;
	u8 *spi_tx;
	int ret;

	/* get pointer to buffers */
	ret = mcp25xxfd_cmd_alloc_buf(spi, tx_len + tx2_len, &spi_tx, NULL);
	if (ret)
		return ret;

	/* setup xfer */
	memset(&xfer, 0, sizeof(xfer));
	xfer.len = tx_len + tx2_len;
	xfer.tx_buf = spi_tx;

	/* copy data to correct location in buffer */
	memcpy(spi_tx, tx_buf, tx_len);
	memcpy(spi_tx + tx_len, tx2_buf, tx2_len);

	/* run the transfer */
	ret = mcp25xxfd_cmd_sync_transfer(spi, &xfer, 1);

	mcp25xxfd_cmd_release_buf(spi, spi_tx, NULL);

	return ret;
}

/* mcp25xxfd spi command/protocol helper */

/* read multiple bytes, transform some registers */
int mcp25xxfd_cmd_readn(struct spi_device *spi, u32 reg,
			void *data, int n)
{
	u8 cmd[2];
	int ret;

	mcp25xxfd_cmd_calc(MCP25XXFD_INSTRUCTION_READ, reg, cmd);

	ret = mcp25xxfd_cmd_write_then_read(spi, &cmd, 2, data, n);
	if (ret)
		return ret;

	return 0;
}

/* read a register, but we are only interrested in a few bytes */
int mcp25xxfd_cmd_read_mask(struct spi_device *spi, u32 reg,
			    u32 *data, u32 mask)
{
	int first_byte, last_byte, len_byte;
	int ret;

	/* check that at least one bit is set */
	if (!mask)
		return -EINVAL;

	/* calculate first and last byte used */
	first_byte = mcp25xxfd_cmd_first_byte(mask);
	last_byte = mcp25xxfd_cmd_last_byte(mask);
	len_byte = last_byte - first_byte + 1;

	/* do a partial read */
	*data = 0;
	ret = mcp25xxfd_cmd_readn(spi, reg + first_byte,
				  ((void *)data + first_byte), len_byte);
	if (ret)
		return ret;

	mcp25xxfd_cmd_convert_to_cpu(data, 1);

	return 0;
}

int mcp25xxfd_cmd_writen(struct spi_device *spi, u32 reg,
			 void *data, int n)
{
	u8 cmd[2];
	int ret;

	mcp25xxfd_cmd_calc(MCP25XXFD_INSTRUCTION_WRITE, reg, cmd);

	ret = mcp25xxfd_cmd_write_then_write(spi, &cmd, 2, data, n);
	if (ret)
		return ret;

	return 0;
}

/* read a register, but we are only interrested in a few bytes */
int mcp25xxfd_cmd_write_mask(struct spi_device *spi, u32 reg,
			     u32 data, u32 mask)
{
	int first_byte, last_byte, len_byte;
	u8 cmd[2];

	/* check that at least one bit is set */
	if (!mask)
		return -EINVAL;

	/* calculate first and last byte used */
	first_byte = mcp25xxfd_cmd_first_byte(mask);
	last_byte = mcp25xxfd_cmd_last_byte(mask);
	len_byte = last_byte - first_byte + 1;

	/* prepare buffer */
	mcp25xxfd_cmd_calc(MCP25XXFD_INSTRUCTION_WRITE,
			   reg + first_byte, cmd);

	mcp25xxfd_cmd_convert_from_cpu(&data, 1);

	return mcp25xxfd_cmd_write_then_write(spi,
					      cmd, sizeof(cmd),
					      ((void *)&data + first_byte),
					      len_byte);
}

int mcp25xxfd_cmd_write_regs(struct spi_device *spi, u32 reg,
			     u32 *data, u32 bytes)
{
	int ret;

	/* first transpose to controller format */
	mcp25xxfd_cmd_convert_from_cpu(data, bytes / sizeof(bytes));

	/* now write it */
	ret = mcp25xxfd_cmd_writen(spi, reg, data, bytes);

	/* and convert it back to cpu format even if it fails */
	mcp25xxfd_cmd_convert_to_cpu(data, bytes / sizeof(bytes));

	return ret;
}

int mcp25xxfd_cmd_read_regs(struct spi_device *spi, u32 reg,
			    u32 *data, u32 bytes)
{
	int ret;

	/* read it */
	ret = mcp25xxfd_cmd_readn(spi, reg, data, bytes);

	/* and convert it to cpu format */
	mcp25xxfd_cmd_convert_to_cpu((u32 *)data, bytes / sizeof(bytes));

	return ret;
}

int mcp25xxfd_cmd_reset(struct spi_device *spi)
{
	u8 *cmd;
	int ret;

	/* allocate 2 bytes on heap, as we use sync_write */
	cmd = kzalloc(2, GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	mcp25xxfd_cmd_calc(MCP25XXFD_INSTRUCTION_RESET, 0, cmd);

	/* write the reset command */
	ret = mcp25xxfd_cmd_sync_write(spi, cmd, 2);

	kfree(cmd);

	return ret;
}
