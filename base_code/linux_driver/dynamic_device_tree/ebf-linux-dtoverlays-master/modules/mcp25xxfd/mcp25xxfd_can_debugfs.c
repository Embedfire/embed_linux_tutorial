// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifdef CONFIG_DEBUG_FS

#include <linux/dcache.h>
#include <linux/debugfs.h>
#include "mcp25xxfd_can_priv.h"
#include "mcp25xxfd_can_tx.h"

static void mcp25xxfd_can_debugfs_regs(struct mcp25xxfd_can_priv *cpriv,
				       struct dentry *root)
{
	struct dentry *dir = debugfs_create_dir("regs", root);

	debugfs_create_x32("con",    0444, dir, &cpriv->regs.con);
	debugfs_create_x32("tdc",    0444, dir, &cpriv->regs.tdc);
	debugfs_create_x32("tscon",  0444, dir, &cpriv->regs.tscon);
	debugfs_create_x32("tefcon", 0444, dir, &cpriv->regs.tscon);
	debugfs_create_x32("nbtcfg", 0444, dir, &cpriv->regs.nbtcfg);
	debugfs_create_x32("dbtcfg", 0444, dir, &cpriv->regs.dbtcfg);
}

static void mcp25xxfd_can_debugfs_status(struct mcp25xxfd_can_priv *cpriv,
					 struct dentry *root)
{
	struct dentry *dir = debugfs_create_dir("status", root);

	debugfs_create_x32("intf",    0444, dir, &cpriv->status.intf);
	debugfs_create_x32("rx_if",   0444, dir, &cpriv->status.rxif);
	debugfs_create_x32("tx_if",   0444, dir, &cpriv->status.txif);
	debugfs_create_x32("rx_ovif", 0444, dir, &cpriv->status.rxovif);
	debugfs_create_x32("tx_atif", 0444, dir, &cpriv->status.txatif);
	debugfs_create_x32("tx_req",  0444, dir, &cpriv->status.txreq);
	debugfs_create_x32("trec",    0444, dir, &cpriv->status.trec);
}

static void mcp25xxfd_can_debugfs_stats(struct mcp25xxfd_can_priv *cpriv,
					struct dentry *root)
{
	struct dentry *dir = debugfs_create_dir("stats", root);
	char name[32];
	u64 *data;
	int i;

# define DEBUGFS_CREATE(name, var) debugfs_create_u64(name, 0444, dir, \
						      &cpriv->stats.var)
	DEBUGFS_CREATE("irq_calls",		 irq_calls);
	DEBUGFS_CREATE("irq_loops",		 irq_loops);
	DEBUGFS_CREATE("irq_thread_rescheduled", irq_thread_rescheduled);

	DEBUGFS_CREATE("int_system_error",	 int_serr_count);
	DEBUGFS_CREATE("int_system_error_tx",	 int_serr_tx_count);
	DEBUGFS_CREATE("int_system_error_rx",	 int_serr_rx_count);
	DEBUGFS_CREATE("int_mode_switch",	 int_mod_count);
	DEBUGFS_CREATE("int_rx",		 int_rx_count);
	DEBUGFS_CREATE("int_tx_attempt",	 int_txat_count);
	DEBUGFS_CREATE("int_tef",		 int_tef_count);
	DEBUGFS_CREATE("int_rx_overflow",	 int_rxov_count);
	DEBUGFS_CREATE("int_ecc_error",		 int_ecc_count);
	DEBUGFS_CREATE("int_rx_invalid_message", int_ivm_count);
	DEBUGFS_CREATE("int_crcerror",		 int_cerr_count);

	DEBUGFS_CREATE("tef_reads",		 tef_reads);
	DEBUGFS_CREATE("tef_conservative_reads", tef_conservative_reads);
	DEBUGFS_CREATE("tef_optimized_reads",	 tef_optimized_reads);
	DEBUGFS_CREATE("tef_read_splits",	 tef_read_splits);

	for (i = 0; i < MCP25XXFD_CAN_TEF_READ_BINS - 1; i++) {
		snprintf(name, sizeof(name),
			 "tef_optimized_reads_%i", i + 1);
		data = &cpriv->stats.tef_optimized_read_sizes[i];
		debugfs_create_u64(name, 0444, dir, data);
	}
	snprintf(name, sizeof(name), "tef_optimized_reads_%i+", i + 1);
	debugfs_create_u64(name, 0444, dir,
			   &cpriv->stats.tef_optimized_read_sizes[i]);

	DEBUGFS_CREATE("tx_frames_fd",		 tx_fd_count);
	DEBUGFS_CREATE("tx_frames_brs",		 tx_brs_count);

	DEBUGFS_CREATE("rx_reads",		 rx_reads);
	DEBUGFS_CREATE("rx_reads_prefetched_too_few",
		       rx_reads_prefetched_too_few);
	DEBUGFS_CREATE("rx_reads_prefetched_too_few_bytes",
		       rx_reads_prefetched_too_few_bytes);
	DEBUGFS_CREATE("rx_reads_prefetched_too_many",
		       rx_reads_prefetched_too_many);
	DEBUGFS_CREATE("rx_reads_prefetched_too_many_bytes",
		       rx_reads_prefetched_too_many_bytes);
	DEBUGFS_CREATE("rx_single_reads",	 rx_single_reads);
	DEBUGFS_CREATE("rx_bulk_reads",		 rx_bulk_reads);

	for (i = 0; i < MCP25XXFD_CAN_RX_BULK_READ_BINS - 1; i++) {
		snprintf(name, sizeof(name), "rx_bulk_reads_%i", i + 1);
		data = &cpriv->stats.rx_bulk_read_sizes[i];
		debugfs_create_u64(name, 0444, dir, data);
	}
	snprintf(name, sizeof(name), "rx_bulk_reads_%i+", i + 1);
	debugfs_create_u64(name, 0444, dir,
			   &cpriv->stats.rx_bulk_read_sizes[i]);

	if (cpriv->can.dev->mtu == CANFD_MTU)
		debugfs_create_u32("rx_reads_prefetch_predicted_len", 0444,
				   dir, &cpriv->rx_history.predicted_len);
#undef DEBUGFS_CREATE
}

static void mcp25xxfd_can_debugfs_tef(struct mcp25xxfd_can_priv *cpriv,
				      struct dentry *root)
{
	struct dentry *dir = debugfs_create_dir("tef", root);

	debugfs_create_u32("count", 0444, dir, &cpriv->fifos.tef.count);
	debugfs_create_u32("size",  0444, dir, &cpriv->fifos.tef.size);
}

static void mcp25xxfd_can_debugfs_fifo_info(struct mcp25xxfd_fifo_info *info,
					    int index, struct dentry *root)
{
	struct dentry *dir;
	char name[4];

	snprintf(name, sizeof(name), "%02i", index);
	dir = debugfs_create_dir(name, root);

	debugfs_create_u32("is_rx",     0444, dir, &info->is_rx);
	debugfs_create_x32("offset",    0444, dir, &info->offset);
	debugfs_create_u32("priority",  0444, dir, &info->priority);

	debugfs_create_u64("use_count", 0444, dir, &info->use_count);
}

static void mcp25xxfd_can_debugfs_fifos(struct mcp25xxfd_can_priv *cpriv,
					struct dentry *root)
{
	struct dentry *dir = debugfs_create_dir("fifos", root);
	int i;

	/* now present all fifos - there is no fifo 0 */
	for (i = 1; i < 32; i++)
		mcp25xxfd_can_debugfs_fifo_info(&cpriv->fifos.info[i], i, dir);
}

static void mcp25xxfd_can_debugfs_rxtx_fifos(struct mcp25xxfd_fifo *d,
					     struct dentry *root)
{
	int i, f;
	char name[4];
	char link[32];

	debugfs_create_u32("count", 0444, root, &d->count);
	debugfs_create_u32("size",  0444, root, &d->size);
	debugfs_create_u32("start", 0444, root, &d->start);

	for (f = d->start, i = 0; i < d->count; f++, i++) {
		snprintf(name, sizeof(name), "%02i", i);
		snprintf(link, sizeof(link), "../fifos/%02i", f);

		debugfs_create_symlink(name, root, link);
	}
}

static void mcp25xxfd_can_debugfs_rx_fifos(struct mcp25xxfd_can_priv *cpriv,
					   struct dentry *root)
{
	struct dentry *dir = debugfs_create_dir("rx_fifos", root);

	mcp25xxfd_can_debugfs_rxtx_fifos(&cpriv->fifos.rx, dir);
}

static void mcp25xxfd_can_debugfs_tx_fifos(struct mcp25xxfd_can_priv *cpriv,
					   struct dentry *root)
{
	struct dentry *dir = debugfs_create_dir("tx_fifos", root);

	mcp25xxfd_can_debugfs_rxtx_fifos(&cpriv->fifos.rx, dir);
}

static void mcp25xxfd_can_debugfs_tx_queue(struct mcp25xxfd_can_priv *cpriv,
					   struct dentry *root)
{
	struct mcp25xxfd_tx_spi_message_queue *queue = cpriv->fifos.tx_queue;
	struct dentry *dir;

	if (!queue)
		return;

	dir = debugfs_create_dir("tx_queue", root);

	debugfs_create_u32("state", 0444, dir, &queue->state);
	debugfs_create_x32("fifos_idle", 0444, dir, &queue->idle);
	debugfs_create_x32("fifos_in_fill_fifo_transfer",
			   0444, dir, &queue->in_fill_fifo_transfer);
	debugfs_create_x32("fifos_in_trigger_fifo_transfer",
			   0444, dir, &queue->in_trigger_fifo_transfer);
	debugfs_create_x32("fifos_in_can_transfer",
			   0444, dir, &queue->in_can_transfer);
	debugfs_create_x32("fifos_transferred",
			   0444, dir, &queue->transferred);
}

void mcp25xxfd_can_debugfs_remove(struct mcp25xxfd_can_priv *cpriv)
{
	debugfs_remove_recursive(cpriv->debugfs_dir);
	cpriv->debugfs_dir = NULL;
}

void mcp25xxfd_can_debugfs_setup(struct mcp25xxfd_can_priv *cpriv)
{
	struct dentry *root;

	/* remove first as we get called during probe and also
	 * when the can_device is configured/removed
	 */
	mcp25xxfd_can_debugfs_remove(cpriv);

	root = debugfs_create_dir("can", cpriv->priv->debugfs_dir);
	cpriv->debugfs_dir = root;

	mcp25xxfd_can_debugfs_regs(cpriv, root);
	mcp25xxfd_can_debugfs_stats(cpriv, root);
	mcp25xxfd_can_debugfs_status(cpriv, root);
	mcp25xxfd_can_debugfs_tef(cpriv, root);
	mcp25xxfd_can_debugfs_fifos(cpriv, root);
	mcp25xxfd_can_debugfs_rx_fifos(cpriv, root);
	mcp25xxfd_can_debugfs_tx_fifos(cpriv, root);
	mcp25xxfd_can_debugfs_tx_queue(cpriv, root);
}

#endif
