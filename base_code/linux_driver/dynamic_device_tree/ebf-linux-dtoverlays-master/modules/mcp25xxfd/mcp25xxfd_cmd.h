/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#ifndef __MCP25XXFD_CMD_H
#define __MCP25XXFD_CMD_H

#include <linux/byteorder/generic.h>
#include <linux/spi/spi.h>
#include <linux/version.h>

/* SPI commands */
#define MCP25XXFD_INSTRUCTION_RESET		0x0000
#define MCP25XXFD_INSTRUCTION_READ		0x3000
#define MCP25XXFD_INSTRUCTION_WRITE		0x2000
#define MCP25XXFD_INSTRUCTION_READ_CRC		0xB000
#define MCP25XXFD_INSTRUCTION_WRITE_CRC		0xA000
#define MCP25XXFD_INSTRUCTION_WRITE_SAVE	0xC000

#define MCP25XXFD_ADDRESS_MASK			0x0fff
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,18,0)
/* XXX: this stuff can be optimized */
static inline void le32_to_cpu_array(u32 *buf, unsigned int words)
{
	while (words--) {
		__le32_to_cpus(buf);
		buf++;
	}
}

static inline void cpu_to_le32_array(u32 *buf, unsigned int words)
{
	while (words--) {
		__cpu_to_le32s(buf);
		buf++;
	}
}

#endif

static inline void mcp25xxfd_cmd_convert_to_cpu(u32 *data, int n)
{
	le32_to_cpu_array(data, n);
}

static inline void mcp25xxfd_cmd_convert_from_cpu(u32 *data, int n)
{
	cpu_to_le32_array(data, n);
}

static inline void mcp25xxfd_cmd_calc(u16 cmd, u16 addr, u8 *data)
{
	cmd = cmd | (addr & MCP25XXFD_ADDRESS_MASK);

	data[0] = (cmd >> 8) & 0xff;
	data[1] = (cmd >> 0) & 0xff;
}

static inline int mcp25xxfd_cmd_first_byte(u32 mask)
{
	return (mask & 0x0000ffff) ?
		((mask & 0x000000ff) ? 0 : 1) :
		((mask & 0x00ff0000) ? 2 : 3);
}

static inline int mcp25xxfd_cmd_last_byte(u32 mask)
{
	return (mask & 0xffff0000) ?
		((mask & 0xff000000) ? 3 : 2) :
		((mask & 0x0000ff00) ? 1 : 0);
}

int mcp25xxfd_cmd_readn(struct spi_device *spi, u32 reg,
			void *data, int n);
int mcp25xxfd_cmd_read_mask(struct spi_device *spi, u32 reg,
			    u32 *data, u32 mask);
static inline int mcp25xxfd_cmd_read(struct spi_device *spi, u32 reg,
				     u32 *data)
{
	return mcp25xxfd_cmd_read_mask(spi, reg, data, -1);
}

int mcp25xxfd_cmd_read_regs(struct spi_device *spi, u32 reg,
			    u32 *data, u32 bytes);

int mcp25xxfd_cmd_writen(struct spi_device *spi, u32 reg,
			 void *data, int n);
int mcp25xxfd_cmd_write_mask(struct spi_device *spi, u32 reg,
			     u32 data, u32 mask);
static inline int mcp25xxfd_cmd_write(struct spi_device *spi, u32 reg,
				      u32 data)
{
	return mcp25xxfd_cmd_write_mask(spi, reg, data, -1);
}

int mcp25xxfd_cmd_write_regs(struct spi_device *spi, u32 reg,
			     u32 *data, u32 bytes);

int mcp25xxfd_cmd_reset(struct spi_device *spi);

#endif /* __MCP25XXFD_CMD_H */
