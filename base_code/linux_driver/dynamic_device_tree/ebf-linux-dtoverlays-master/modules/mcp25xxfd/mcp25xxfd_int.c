// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#include <linux/kernel.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_can_int.h"
#include "mcp25xxfd_crc.h"
#include "mcp25xxfd_ecc.h"
#include "mcp25xxfd_int.h"
#include "mcp25xxfd_priv.h"

int mcp25xxfd_int_clear(struct mcp25xxfd_priv *priv)
{
	int ret;

	ret = mcp25xxfd_ecc_clear_int(priv);
	if (ret)
		return ret;
	ret = mcp25xxfd_crc_clear_int(priv);
	if (ret)
		return ret;
	ret = mcp25xxfd_can_int_clear(priv);

	return ret;
}

int mcp25xxfd_int_enable(struct mcp25xxfd_priv *priv, bool enable)
{
	/* error handling only on enable for this function */
	int ret = 0;

	/* if we enable clear interrupt flags first */
	if (enable) {
		ret = mcp25xxfd_int_clear(priv);
		if (ret)
			goto out;
	}

	ret = mcp25xxfd_crc_enable_int(priv, enable);
	if (ret)
		goto out;

	ret = mcp25xxfd_ecc_enable(priv);
	if (ret)
		goto out_crc;

	ret = mcp25xxfd_ecc_enable_int(priv, enable);
	if (ret)
		goto out_crc;

	ret = mcp25xxfd_can_int_enable(priv, enable);
	if (ret)
		goto out_ecc;

	/* if we disable interrupts clear interrupt flags last */
	if (!enable)
		mcp25xxfd_int_clear(priv);

	return 0;

out_ecc:
	mcp25xxfd_ecc_enable_int(priv, false);

out_crc:
	mcp25xxfd_crc_enable_int(priv, false);
out:
	return ret;
}
