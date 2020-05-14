/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CAN_DEBUGFS_H
#define __MCP25XXFD_CAN_DEBUGFS_H

#ifdef CONFIG_DEBUG_FS

#include <linux/debugfs.h>
#include "mcp25xxfd_can_priv.h"

#define MCP25XXFD_DEBUGFS_INCR(counter) ((counter)++)
#define MCP25XXFD_DEBUGFS_ADD(counter, val) ((counter) += (val))
#define MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, counter)		\
	(((cpriv)->stats.counter)++)
#define MCP25XXFD_DEBUGFS_STATS_ADD(cpriv, counter, val)	\
	(((cpriv)->stats.counter) += (val))

void mcp25xxfd_can_debugfs_setup(struct mcp25xxfd_can_priv *cpriv);
void mcp25xxfd_can_debugfs_remove(struct mcp25xxfd_can_priv *cpriv);

#else /* CONFIG_DEBUG_FS */

#define MCP25XXFD_DEBUGFS_INCR(counter)
#define MCP25XXFD_DEBUGFS_ADD(counter, val)
#define MCP25XXFD_DEBUGFS_STATS_INCR(cpriv, counter)
#define MCP25XXFD_DEBUGFS_STATS_ADD(cpriv, counter, val)

static inline
void mcp25xxfd_can_debugfs_setup(struct mcp25xxfd_can_priv *cpriv)
{
}

static inline
void mcp25xxfd_can_debugfs_remove(struct mcp25xxfd_can_priv *cpriv)
{
}

#endif /* CONFIG_DEBUG_FS */
#endif /* __MCP25XXFD_CAN_DEBUGFS_H */
