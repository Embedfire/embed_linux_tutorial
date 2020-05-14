// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "mcp25xxfd_can.h"
#include "mcp25xxfd_clock.h"
#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_debugfs.h"
#include "mcp25xxfd_ecc.h"
#include "mcp25xxfd_gpio.h"
#include "mcp25xxfd_int.h"
#include "mcp25xxfd_priv.h"

/* Device description and rational:
 *
 * The mcp25xxfd is a CanFD controller that also supports can2.0 only
 * modes.
 * It is connected via spi to the host and requires at minimum a single
 * irq line in addition to the SPI lines - it is not mentioned explicitly
 * in the documentation but in principle SPI 3-wire should be possible.
 *
 * The clock connected is typically 4MHz, 20MHz or 40MHz.
 * When using a 4MHz clock the controller can use an integrated PLL to
 * get 40MHz.
 *
 * The controller itself has 2KB of SRAM for CAN-data.
 * ECC can get enabled for SRAM.
 * CRC-16 checksumming of SPI transfers can get implemented
 *   - some optimization options may not be efficient in such a situation.
 *   - more SPI bus bandwidth is used for transfer of CRCs and
 *     transfer length information
 *
 * It also contains 2 GPIO pins that can get used either as interrupt lines
 * or GPIO IN or Out or STANDBY flags.
 * In addition there is a PIN that allows output of a (divided) clock out
 * or as a SOF (Start of Can FRAME) interrupt line - e.g for wakeup.
 */

int mcp25xxfd_base_power_enable(struct regulator *reg, int enable)
{
	if (IS_ERR_OR_NULL(reg))
		return 0;

	if (enable)
		return regulator_enable(reg);
	else
		return regulator_disable(reg);
}

static const struct of_device_id mcp25xxfd_of_match[] = {
	{
		.compatible	= "microchip,mcp2517fd",
		.data		= (void *)CAN_MCP2517FD,
	},
	{ }
};
MODULE_DEVICE_TABLE(of, mcp25xxfd_of_match);

static int mcp25xxfd_base_probe(struct spi_device *spi)
{
	const struct of_device_id *of_id =
		of_match_device(mcp25xxfd_of_match, &spi->dev);
	struct mcp25xxfd_priv *priv;
	int ret;

	/* as irq_create_fwspec_mapping() can return 0, check for it */
	if (spi->irq <= 0) {
		dev_err(&spi->dev, "no valid irq line defined: irq = %i\n",
			spi->irq);
		return -EINVAL;
	}

	priv = devm_kzalloc(&spi->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* cross assigns */
	spi_set_drvdata(spi, priv);
	priv->spi = spi;

	/* assign name */
	snprintf(priv->device_name, sizeof(priv->device_name),
		 DEVICE_NAME "-%s", dev_name(&priv->spi->dev));

	/* assign model from of or driver_data */
	if (of_id)
		priv->model = (enum mcp25xxfd_model)of_id->data;
	else
		priv->model = spi_get_device_id(spi)->driver_data;

	mutex_init(&priv->spi_rxtx_lock);

	ret = mcp25xxfd_clock_init(priv);
	if (ret)
		goto out_free;

	/* Configure the SPI bus */
	spi->bits_per_word = 8;
	ret = spi_setup(spi);
	if (ret)
		goto out_clk;

	priv->power = devm_regulator_get_optional(&spi->dev, "vdd");
	if (PTR_ERR(priv->power) == -EPROBE_DEFER) {
		ret = -EPROBE_DEFER;
		goto out_clk;
	}

	ret = mcp25xxfd_base_power_enable(priv->power, 1);
	if (ret)
		goto out_clk;

	/* this will also enable the MCP25XXFD_CLK_USER_CAN clock */
	ret = mcp25xxfd_clock_probe(priv);
	if (ret)
		goto out_probe;

	/* enable the can controller clock */
	ret = mcp25xxfd_clock_start(priv, MCP25XXFD_CLK_USER_CAN);
	if (ret)
		goto out_probe;

	/* try to identify the can-controller - we need the clock here */
	ret = mcp25xxfd_can_probe(priv);
	if (ret)
		goto out_ctlclk;

	/* add debugfs */
	mcp25xxfd_debugfs_setup(priv);

	/* disable interrupts */
	ret = mcp25xxfd_int_enable(priv, false);
	if (ret)
		goto out_debugfs;

	/* setup ECC for SRAM */
	ret = mcp25xxfd_ecc_enable(priv);
	if (ret)
		goto out_debugfs;

	/* setting up GPIO */
	ret = mcp25xxfd_gpio_setup(priv);
	if (ret)
		goto out_debugfs;

	/* setting up CAN */
	ret = mcp25xxfd_can_setup(priv);
	if (ret)
		goto out_gpio;

	/* and put controller to sleep by stopping the can clock */
	ret = mcp25xxfd_clock_stop(priv, MCP25XXFD_CLK_USER_CAN);
	if (ret)
		goto out_can;

	dev_info(&spi->dev,
		 "MCP%x successfully initialized.\n", priv->model);
	return 0;

out_can:
	mcp25xxfd_can_remove(priv);
out_gpio:
	mcp25xxfd_gpio_remove(priv);
out_debugfs:
	mcp25xxfd_debugfs_remove(priv);
out_ctlclk:
	mcp25xxfd_clock_stop(priv, MCP25XXFD_CLK_USER_CAN);
out_probe:
	mcp25xxfd_base_power_enable(priv->power, 0);
out_clk:
	mcp25xxfd_clock_release(priv);
out_free:
	dev_err(&spi->dev, "Probe failed, err=%d\n", -ret);
	return ret;
}

static int mcp25xxfd_base_remove(struct spi_device *spi)
{
	struct mcp25xxfd_priv *priv = spi_get_drvdata(spi);

	/* remove can */
	mcp25xxfd_can_remove(priv);

	/* remove gpio */
	mcp25xxfd_gpio_remove(priv);

	/* clear all running clocks */
	mcp25xxfd_clock_stop(priv, priv->clk_user_mask);

	mcp25xxfd_debugfs_remove(priv);

	mcp25xxfd_base_power_enable(priv->power, 0);

	mcp25xxfd_clock_release(priv);

	return 0;
}

static int __maybe_unused mcp25xxfd_base_suspend(struct device *dev)
{
	struct spi_device *spi = to_spi_device(dev);
	struct mcp25xxfd_priv *priv = spi_get_drvdata(spi);

	mutex_lock(&priv->clk_user_lock);
	priv->clk_sleep_mask = priv->clk_user_mask;
	mutex_unlock(&priv->clk_user_lock);

	/* disable interrupts */
	mcp25xxfd_int_enable(priv, false);

	/* stop the clocks */
	mcp25xxfd_clock_stop(priv, priv->clk_sleep_mask);

	/* disable power to controller */
	return mcp25xxfd_base_power_enable(priv->power, 0);
}

static int __maybe_unused mcp25xxfd_base_resume(struct device *dev)
{
	struct spi_device *spi = to_spi_device(dev);
	struct mcp25xxfd_priv *priv = spi_get_drvdata(spi);
	int ret = 0;

	/* enable power to controller */
	mcp25xxfd_base_power_enable(priv->power, 1);

	/* if there is no sleep mask, then there is nothing to wake */
	if (!priv->clk_sleep_mask)
		return 0;

	/* start the clocks */
	ret = mcp25xxfd_clock_start(priv, priv->clk_sleep_mask);
	if (ret)
		return 0;

	/* clear the sleep mask */
	mutex_lock(&priv->clk_user_lock);
	priv->clk_sleep_mask = 0;
	mutex_unlock(&priv->clk_user_lock);

	/* enable the interrupts again */
	return mcp25xxfd_int_enable(priv, true);
}

static SIMPLE_DEV_PM_OPS(mcp25xxfd_base_pm_ops, mcp25xxfd_base_suspend,
			 mcp25xxfd_base_resume);

static const struct spi_device_id mcp25xxfd_id_table[] = {
	{
		.name		= "mcp2517fd",
		.driver_data	= (kernel_ulong_t)CAN_MCP2517FD,
	},
	{ }
};
MODULE_DEVICE_TABLE(spi, mcp25xxfd_id_table);

static struct spi_driver mcp25xxfd_can_driver = {
	.driver = {
		.name = DEVICE_NAME,
		.of_match_table = mcp25xxfd_of_match,
		.pm = &mcp25xxfd_base_pm_ops,
	},
	.id_table = mcp25xxfd_id_table,
	.probe = mcp25xxfd_base_probe,
	.remove = mcp25xxfd_base_remove,
};
module_spi_driver(mcp25xxfd_can_driver);

MODULE_AUTHOR("Martin Sperl <kernel@martin.sperl.org>");
MODULE_DESCRIPTION("Microchip 25XXFD CAN driver");
MODULE_LICENSE("GPL v2");
