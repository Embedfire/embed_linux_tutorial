/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_DEBUGFS_H
#define __MCP25XXFD_DEBUGFS_H

#include "mcp25xxfd_priv.h"

#ifdef CONFIG_DEBUG_FS

void mcp25xxfd_debugfs_setup(struct mcp25xxfd_priv *priv);
void mcp25xxfd_debugfs_remove(struct mcp25xxfd_priv *priv);

#else

static inline void mcp25xxfd_debugfs_setup(struct mcp25xxfd_priv *priv)
{
	return 0;
}

static inline void mcp25xxfd_debugfs_remove(struct mcp25xxfd_priv *priv)
{
}

#endif /* CONFIG_DEBUG_FS */
#endif /* __MCP25XXFD_DEBUGFS_H */
