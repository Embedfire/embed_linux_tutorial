// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifdef CONFIG_DEBUG_FS

#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>

#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_debugfs.h"
#include "mcp25xxfd_priv.h"

static int mcp25xxfd_debugfs_dump_regs_range(struct seq_file *file,
					     u32 start, u32 end)
{
	struct spi_device *spi = file->private;
	u32 data[32];
	int bytes = end - start + sizeof(*data);
	int i, l, count, ret;

	for (count =  bytes / sizeof(*data); count > 0; count -= 32) {
		/* read up to 32 registers in one go */
		l = min(count, 32);
		ret = mcp25xxfd_cmd_read_regs(spi, start,
					      data, l * sizeof(*data));
		if (ret)
			return ret;
		/* dump those read registers */
		for (i = 0; i < l; i++, start += sizeof(*data))
			seq_printf(file, "Reg 0x%03x = 0x%08x\n",
				   start, data[i]);
	}

	return 0;
}

static int mcp25xxfd_debugfs_dump_regs(struct seq_file *file, void *offset)
{
	return mcp25xxfd_debugfs_dump_regs_range(file, MCP25XXFD_OSC,
						 MCP25XXFD_ECCSTAT);
}

static int mcp25xxfd_debugfs_dump_can_regs(struct seq_file *file,
					   void *offset)
{
	return mcp25xxfd_debugfs_dump_regs_range(file, MCP25XXFD_CAN_CON,
						 MCP25XXFD_CAN_TXQUA);
}

static int mcp25xxfd_debugfs_dump_can_all_regs(struct seq_file *file,
					       void *offset)
{
	return mcp25xxfd_debugfs_dump_regs_range(file, MCP25XXFD_CAN_CON,
						 MCP25XXFD_CAN_FLTMASK(31));
}

static void mcp25xxfd_debugfs_mod_setup(struct mcp25xxfd_priv *priv)
{
	struct dentry *root, *regs;

	/* the base directory */
	priv->debugfs_dir = debugfs_create_dir(priv->device_name, NULL);
	root = priv->debugfs_dir;

	/* expose some parameters related to clocks */
	debugfs_create_u32("spi_setup_speed_hz", 0444, root,
			   &priv->spi_setup_speed_hz);
	debugfs_create_u32("spi_normal_speed_hz", 0444, root,
			   &priv->spi_normal_speed_hz);
	debugfs_create_u32("spi_use_speed_hz", 0444, root,
			   &priv->spi_use_speed_hz);
	debugfs_create_u32("clk_user_mask", 0444, root, &priv->clk_user_mask);

	/* expose the system registers */
	priv->debugfs_regs_dir = debugfs_create_dir("regs", root);
	regs = priv->debugfs_regs_dir;
	debugfs_create_x32("osc", 0444, regs, &priv->regs.osc);
	debugfs_create_x32("iocon", 0444, regs, &priv->regs.iocon);
	debugfs_create_x32("crc", 0444, regs, &priv->regs.crc);
	debugfs_create_x32("ecccon", 0444, regs, &priv->regs.ecccon);

	/* dump the controller registers themselves */
	debugfs_create_devm_seqfile(&priv->spi->dev, "regs_live_dump",
				    root, mcp25xxfd_debugfs_dump_regs);
	/* and the essential can registers */
	debugfs_create_devm_seqfile(&priv->spi->dev, "can_regs_live_dump",
				    root, mcp25xxfd_debugfs_dump_can_regs);
	/* and the complete can registers */
	debugfs_create_devm_seqfile(&priv->spi->dev,
				    "can_regs_all_live_dump", root,
				    mcp25xxfd_debugfs_dump_can_all_regs);
}

void mcp25xxfd_debugfs_setup(struct mcp25xxfd_priv *priv)
{
	mcp25xxfd_debugfs_mod_setup(priv);
}

void mcp25xxfd_debugfs_remove(struct mcp25xxfd_priv *priv)
{
	debugfs_remove_recursive(priv->debugfs_dir);
	priv->debugfs_dir = NULL;
}

#endif /* CONFIG_DEBUG_FS */
