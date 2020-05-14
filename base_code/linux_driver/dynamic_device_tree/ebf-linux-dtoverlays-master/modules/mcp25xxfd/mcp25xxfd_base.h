/* SPDX-License-Identifier: GPL-2.0 */

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */
#ifndef __MCP25XXFD_BASE_H
#define __MCP25XXFD_BASE_H

#include <linux/regulator/consumer.h>

int mcp25xxfd_base_power_enable(struct regulator *reg, int enable);

#endif /* __MCP25XXFD_BASE_H */
