/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CAN_PRIV_H
#define __MCP25XXFD_CAN_PRIV_H

#include <linux/can/dev.h>
#include <linux/dcache.h>

#include "mcp25xxfd_priv.h"

#define TX_ECHO_SKB_MAX	32

/* information on each fifo type */
struct mcp25xxfd_fifo {
	u32 count;
	u32 start;
	u32 size;
#ifdef CONFIG_DEBUG_FS
	u64 dlc_usage[16];
	u64 fd_count;
#endif /* CONFIG_DEBUG_FS */
};

/* used for sorting incoming messages */
struct mcp25xxfd_obj_ts {
	s32 ts; /* using signed to handle rollover correctly when sorting */
	u16 fifo;
	s16 is_rx;
};

/* general info on each fifo */
struct mcp25xxfd_fifo_info {
	u32 is_rx;
	u32 offset;
	u32 priority;
#ifdef CONFIG_DEBUG_FS
	u64 use_count;
#endif /* CONFIG_DEBUG_FS */
};

struct mcp25xxfd_can_priv {
	/* can_priv has to be the first one to be usable with alloc_candev
	 * which expects struct can_priv to be right at the start of the
	 * priv structure
	 */
	struct can_priv can;
	struct mcp25xxfd_priv *priv;
	struct regulator *transceiver;

	/* the can mode currently active */
	int mode;

	/* interrupt state */
	struct {
		int enabled;
		int allocated;
	} irq;

	/* can config registers */
	struct {
		u32 con;
		u32 tdc;
		u32 tscon;
		u32 tefcon;
		u32 nbtcfg;
		u32 dbtcfg;
	} regs;

	/* can status registers (mostly) - read in one go
	 * bdiag0 and bdiag1 are optional, but when
	 * berr counters are requested on a regular basis
	 * during high CAN-bus load this would trigger the fact
	 * that spi_sync would get queued for execution in the
	 * spi thread and the spi handler would not get
	 * called inline in the interrupt thread without any
	 * context switches or wakeups...
	 */
	struct {
		u32 intf;
		/* ASSERT(CAN_INT + 4 == CAN_RXIF) */
		u32 rxif;
		/* ASSERT(CAN_RXIF + 4 == CAN_TXIF) */
		u32 txif;
		/* ASSERT(CAN_TXIF + 4 == CAN_RXOVIF) */
		u32 rxovif;
		/* ASSERT(CAN_RXOVIF + 4 == CAN_TXATIF) */
		u32 txatif;
		/* ASSERT(CAN_TXATIF + 4 == CAN_TXREQ) */
		u32 txreq;
		/* ASSERT(CAN_TXREQ + 4 == CAN_TREC) */
		u32 trec;
	} status;

	/* information of fifo setup */
	struct {
		/* define payload size and mode */
		u32 payload_size;
		u32 payload_mode;

		/* infos on fifo layout */

		/* TEF */
		struct {
			u32 count;
			u32 size;
			u32 index;
		} tef;

		/* info on each fifo */
		struct mcp25xxfd_fifo_info info[32];

		/* extra info on rx/tx fifo groups */
		struct mcp25xxfd_fifo tx;
		struct mcp25xxfd_fifo rx;

		/* queue of can frames that need to get submitted
		 * to the network stack during an interrupt loop in one go
		 * (this gets sorted by timestamp before submission
		 * and contains both rx frames as well tx frames that have
		 * gone over the CAN bus successfully
		 */
		struct mcp25xxfd_obj_ts submit_queue[32];
		int  submit_queue_count;

		/* the tx queue of spi messages */
		struct mcp25xxfd_tx_spi_message_queue *tx_queue;
	} fifos;

	/* statistics exposed via debugfs */
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_dir;

	struct {
		u64 irq_calls;
		u64 irq_loops;
		u64 irq_thread_rescheduled;

		u64 int_serr_count;
		u64 int_serr_rx_count;
		u64 int_serr_tx_count;
		u64 int_mod_count;
		u64 int_rx_count;
		u64 int_txat_count;
		u64 int_tef_count;
		u64 int_rxov_count;
		u64 int_ecc_count;
		u64 int_ivm_count;
		u64 int_cerr_count;

		u64 tx_fd_count;
		u64 tx_brs_count;

		u64 tef_reads;
		u64 tef_read_splits;
		u64 tef_conservative_reads;
		u64 tef_optimized_reads;
#define MCP25XXFD_CAN_TEF_READ_BINS 8
		u64 tef_optimized_read_sizes[MCP25XXFD_CAN_TEF_READ_BINS];

		u64 rx_reads;
		u64 rx_reads_prefetched_too_few;
		u64 rx_reads_prefetched_too_few_bytes;
		u64 rx_reads_prefetched_too_many;
		u64 rx_reads_prefetched_too_many_bytes;
		u64 rx_single_reads;
		u64 rx_bulk_reads;
#define MCP25XXFD_CAN_RX_BULK_READ_BINS 8
		u64 rx_bulk_read_sizes[MCP25XXFD_CAN_RX_BULK_READ_BINS];
	} stats;
#endif /* CONFIG_DEBUG_FS */

	/* history of rx-dlc */
	struct {
#define MCP25XXFD_CAN_RX_DLC_HISTORY_SIZE 32
		u8 dlc[MCP25XXFD_CAN_RX_DLC_HISTORY_SIZE];
		u8 brs[MCP25XXFD_CAN_RX_DLC_HISTORY_SIZE];
		u8 index;
		u32 predicted_len;
	} rx_history;

	/* bus state */
	struct {
		u32 state;
		u32 new_state;
		u32 bdiag[2];
	} bus;

	/* can error messages */
	struct {
		u32 id;
		u8  data[8];
	} error_frame;

	/* a copy of mcp25xxfd-sram in ram */
	u8 sram[MCP25XXFD_SRAM_SIZE];
};

#endif /* __MCP25XXFD_CAN_PRIV_H */
