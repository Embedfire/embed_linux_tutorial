// SPDX-License-Identifier: GPL-2.0

/* CAN bus driver for Microchip 25XXFD CAN Controller with SPI Interface
 *
 * Copyright 2019 Martin Sperl <kernel@martin.sperl.org>
 */

/* Known hardware issues and workarounds in this driver:
 *
 * * There is one situation where the controller will require a full POR
 *   (total power off) to recover from a bad Clock configuration.
 *   This happens when the wrong clock is configured in the device tree
 *   (say 4MHz are configured, while 40MHz is the actual clock frequency
 *   of the HW).
 *   In such a situation the driver tries to enable the PLL, which will
 *   never synchronize and the controller becomes unresponsive to further
 *   spi requests until a full POR.
 *
 *   Mitigation:
 *     none as of now
 *
 *   Possible implementation of a mitigation/sanity check:
 *     during initialization:
 *       * try to identify the HW at 1MHz:
 *         on success:
 *           * controller is identified
 *         on failure:
 *           * controller is absent - fail
 *       * force controller clock to run with disabled PLL
 *       * try to identify the HW at 2MHz:
 *         on success:
 *           * controller clock is >= 4 MHz
 *           * this may be 4MHz
 *         on failure:
 *           * controller clock is < 4 MHz
 *       * try to identify the HW at 2.5MHz:
 *         on success:
 *           * controller clock is >= 5 MHz
 *           * this may not be 4MHz
 *         on failure:
 *           * controller clock is 4 MHz
 *           * enable PLL
 *           * exit successfully (or run last test for verification purposes)
 *       * try to identify the HW at <dt-clock/2> MHz:
 *         on success:
 *           * controller clock is >= <dt-clock/2> MHz
 *              (it could be higher though)
 *         on failure:
 *           * the controller is not running at the
 *             clock rate configured in the DT
 *           * if PLL is enabled warn about requirements of POR
 *           * fail
 *
 *   Side-effects:
 *     * longer initialization time
 *
 *   Possible issues with mitigation:
 *     * possibly miss-identification because the SPI block may work
 *       "somewhat" at frequencies > < clock / 2 + delta f>
 *       this may be especially true for the situation where we test if
 *       2.5MHz SPI-Clock works.
 *     * also SPI HW-clock dividers may do a round down to fixed frequencies
 *       which is not properly reported and may result in false positives
 *       because a frequency lower than expected is used.
 *
 *   This is the reason why only simple testing is enabled at the risk of
 *   the need for a POR.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/spi/spi.h>

#include "mcp25xxfd_can.h"
#include "mcp25xxfd_cmd.h"
#include "mcp25xxfd_priv.h"

/* the PLL may take some time to synchronize - use 1 second as timeout */
#define MCP25XXFD_OSC_POLLING_JIFFIES	(HZ)

static u32 _mcp25xxfd_clkout_mask(struct mcp25xxfd_priv *priv)
{
	u32 val = 0;

	if (priv->config.clock_div2)
		val |= MCP25XXFD_OSC_SCLKDIV;

	switch (priv->config.clock_odiv) {
	case 0:
		break;
	case 1:
		val |= MCP25XXFD_OSC_CLKODIV_1 << MCP25XXFD_OSC_CLKODIV_SHIFT;
		break;
	case 2:
		val |= MCP25XXFD_OSC_CLKODIV_2 << MCP25XXFD_OSC_CLKODIV_SHIFT;
		break;
	case 4:
		val |= MCP25XXFD_OSC_CLKODIV_4 << MCP25XXFD_OSC_CLKODIV_SHIFT;
		break;
	case 10:
		val |= MCP25XXFD_OSC_CLKODIV_10 << MCP25XXFD_OSC_CLKODIV_SHIFT;
		break;
	default:
		/* this should never happen but is error-handled
		 * by the dt-parsing
		 */
		break;
	}

	return val;
}

static int _mcp25xxfd_waitfor_osc(struct mcp25xxfd_priv *priv,
				  u32 waitfor, u32 mask)
{
	unsigned long timeout;
	int ret;

	/* wait for synced pll/osc/sclk */
	timeout = jiffies + MCP25XXFD_OSC_POLLING_JIFFIES;
	while (time_before_eq(jiffies, timeout)) {
		ret = mcp25xxfd_cmd_read(priv->spi, MCP25XXFD_OSC,
					 &priv->regs.osc);
		if (ret)
			return ret;
		/* check for expected bits to be set/unset */
		if ((priv->regs.osc & mask) == waitfor)
			return 0;
	}

	return -ETIMEDOUT;
}

static int _mcp25xxfd_clock_configure_osc(struct mcp25xxfd_priv *priv,
					  u32 value, u32 waitfor, u32 mask)
{
	int ret;

	/* write the osc value to the controller - waking it if necessary */
	ret = mcp25xxfd_cmd_write(priv->spi, MCP25XXFD_OSC, value);
	if (ret)
		return ret;

	/* wait for the clock to stabelize */
	ret = _mcp25xxfd_waitfor_osc(priv, waitfor, mask);

	/* on timeout try again setting the register */
	if (ret == -ETIMEDOUT) {
		/* write the clock to the controller */
		ret = mcp25xxfd_cmd_write(priv->spi, MCP25XXFD_OSC, value);
		if (ret)
			return ret;

		/* wait for the clock to stabelize */
		ret = _mcp25xxfd_waitfor_osc(priv, waitfor, mask);
	}

	/* handle timeout special - report the fact */
	if (ret == -ETIMEDOUT)
		dev_err(&priv->spi->dev,
			"Clock did not switch within the timeout period\n");

	return ret;
}

static int _mcp25xxfd_clock_start(struct mcp25xxfd_priv *priv)
{
	u32 value = _mcp25xxfd_clkout_mask(priv);
	u32 waitfor = MCP25XXFD_OSC_OSCRDY;
	u32 mask = waitfor | MCP25XXFD_OSC_OSCDIS | MCP25XXFD_OSC_PLLRDY |
		MCP25XXFD_OSC_PLLEN;

	/* enable PLL as well - set expectations */
	if (priv->config.clock_pll) {
		value   |= MCP25XXFD_OSC_PLLEN;
		waitfor |= MCP25XXFD_OSC_PLLRDY | MCP25XXFD_OSC_PLLEN;
	}

	/* set the oscilator now */
	return _mcp25xxfd_clock_configure_osc(priv, value, waitfor, mask);
}

static int _mcp25xxfd_clock_stop(struct mcp25xxfd_priv *priv)
{
	u32 value = _mcp25xxfd_clkout_mask(priv);
	u32 waitfor = 0;
	u32 mask = MCP25XXFD_OSC_OSCDIS | MCP25XXFD_OSC_PLLRDY |
		MCP25XXFD_OSC_PLLEN;
	int ret;

	ret = _mcp25xxfd_clock_configure_osc(priv, value, waitfor, mask);
	if (ret)
		return ret;

	/* finally switch the controller mode to sleep
	 * by this time the controller should be in config mode already
	 * this way we wake to config mode again
	 */
	return mcp25xxfd_can_sleep_mode(priv);
}

int mcp25xxfd_clock_start(struct mcp25xxfd_priv *priv, int requestor_mask)
{
	int ret = 0;

	/* without a clock there is nothing we can do... */
	if (IS_ERR(priv->clk))
		return PTR_ERR(priv->clk);

	mutex_lock(&priv->clk_user_lock);

	/* if clock is already started, then skip */
	if (priv->clk_user_mask & requestor_mask)
		goto out;

	/* enable the clock on the host side*/
	ret = clk_prepare_enable(priv->clk);
	if (ret)
		goto out;

	/* enable the clock on the controller side */
	ret = _mcp25xxfd_clock_start(priv);
	if (ret)
		goto out;

	/* mark the clock for the specific component as started */
	priv->clk_user_mask |= requestor_mask;

	/* and now we use the normal spi speed */
	priv->spi_use_speed_hz = priv->spi_normal_speed_hz;

out:
	mutex_unlock(&priv->clk_user_lock);

	return ret;
}

int mcp25xxfd_clock_stop(struct mcp25xxfd_priv *priv, int requestor_mask)
{
	int ret;

	/* without a clock there is nothing we can do... */
	if (IS_ERR(priv->clk))
		return PTR_ERR(priv->clk);

	mutex_lock(&priv->clk_user_lock);

	/* if the mask is empty then skip, as the clock is stopped */
	if (!priv->clk_user_mask)
		goto out;

	/* clear the clock mask */
	priv->clk_user_mask &= ~requestor_mask;

	/* if the mask is not empty then skip, as the clock is needed */
	if (priv->clk_user_mask)
		goto out;

	/* and now we use the setup spi speed */
	priv->spi_use_speed_hz = priv->spi_setup_speed_hz;

	/* stop the clock on the controller */
	ret = _mcp25xxfd_clock_stop(priv);

	/* and we stop the clock on the host*/
	if (!IS_ERR(priv->clk))
		clk_disable_unprepare(priv->clk);
out:
	mutex_unlock(&priv->clk_user_lock);

	return 0;
}

static int _mcp25xxfd_clock_probe(struct mcp25xxfd_priv *priv)
{
	int ret;

	/* Wait for oscillator startup timer after power up */
	mdelay(MCP25XXFD_OST_DELAY_MS);

	/* send a "blind" reset, hoping we are in Config mode */
	mcp25xxfd_cmd_reset(priv->spi);

	/* Wait for oscillator startup again */
	mdelay(MCP25XXFD_OST_DELAY_MS);

	/* check clock register that the clock is ready or disabled */
	ret = mcp25xxfd_cmd_read(priv->spi, MCP25XXFD_OSC,
				 &priv->regs.osc);
	if (ret)
		return ret;

	/* there can only be one... */
	switch (priv->regs.osc &
		(MCP25XXFD_OSC_OSCRDY | MCP25XXFD_OSC_OSCDIS)) {
	case MCP25XXFD_OSC_OSCRDY: /* either the clock is ready */
		break;
	case MCP25XXFD_OSC_OSCDIS: /* or the clock is disabled */
		break;
	default:
		/* otherwise there is no valid device (or in strange state)
		 *
		 * if PLL is enabled but not ready, then there may be
		 * something "fishy"
		 * this happened during driver development
		 * (enabling pll, when when on wrong clock), so best warn
		 * about such a possibility
		 */
		if ((priv->regs.osc &
		     (MCP25XXFD_OSC_PLLEN | MCP25XXFD_OSC_PLLRDY))
		    == MCP25XXFD_OSC_PLLEN)
			dev_err(&priv->spi->dev,
				"mcp25xxfd may be in a strange state - a power disconnect may be required\n");

		return -ENODEV;
	}

	return 0;
}

int mcp25xxfd_clock_probe(struct mcp25xxfd_priv *priv)
{
	int ret;

	/* this will also enable the MCP25XXFD_CLK_USER_CAN clock */
	ret = _mcp25xxfd_clock_probe(priv);

	/* on error retry a second time */
	if (ret == -ENODEV) {
		ret = _mcp25xxfd_clock_probe(priv);
		if (!ret)
			dev_info(&priv->spi->dev,
				 "found device only during retry\n");
	}
	if (ret) {
		if (ret == -ENODEV)
			dev_err(&priv->spi->dev,
				"Cannot initialize MCP%x. Wrong wiring? (oscilator register reads as %08x)\n",
				priv->model, priv->regs.osc);
	}

	return ret;
}

void mcp25xxfd_clock_release(struct mcp25xxfd_priv *priv)
{
	if (!IS_ERR_OR_NULL(priv->clk))
		clk_disable_unprepare(priv->clk);
}

#ifdef CONFIG_OF_DYNAMIC
static int mcp25xxfd_clock_of_parse(struct mcp25xxfd_priv *priv)
{
	struct spi_device *spi = priv->spi;
	const struct device_node *np = spi->dev.of_node;
	u32 val;
	int ret;

	priv->config.clock_div2 = false;
	priv->config.clock_div2 =
		of_property_read_bool(np, "microchip,clock-div2");

	priv->config.clock_odiv = 10;
	ret = of_property_read_u32_index(np, "microchip,clock-out-div",
					 0, &val);
	if (!ret) {
		switch (val) {
		case 0:
		case 1:
		case 2:
		case 4:
		case 10:
			priv->config.clock_odiv = val;
			break;
		default:
			dev_err(&spi->dev,
				"Invalid value in device tree for microchip,clock_out_div: %u - valid values: 0, 1, 2, 4, 10\n",
				val);
			return -EINVAL;
		}
	}

	return 0;
}
#else
static int mcp25xxfd_clock_of_parse(struct mcp25xxfd_priv *priv)
{
	return 0;
}
#endif

int mcp25xxfd_clock_init(struct mcp25xxfd_priv *priv)
{
	struct spi_device *spi = priv->spi;
	struct clk *clk;
	int ret, freq;

	mutex_init(&priv->clk_user_lock);

	priv->config.clock_div2 = false;
	priv->config.clock_odiv = 10;

	ret = mcp25xxfd_clock_of_parse(priv);
	if (ret)
		return ret;

	/* get clock */
	clk = devm_clk_get(&spi->dev, NULL);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	freq = clk_get_rate(clk);
	if (freq < MCP25XXFD_MIN_CLOCK_FREQUENCY ||
	    freq > MCP25XXFD_MAX_CLOCK_FREQUENCY) {
		dev_err(&spi->dev,
			"Clock frequency %i is not in range [%i:%i]\n",
			freq,
			MCP25XXFD_MIN_CLOCK_FREQUENCY,
			MCP25XXFD_MAX_CLOCK_FREQUENCY);
		return -ERANGE;
	}

	/* enable the clock and mark as enabled */
	ret = clk_prepare_enable(clk);
	if (ret)
		return ret;
	priv->clk = clk;

	/* if we have a clock that is <= 4MHz, enable the pll */
	priv->config.clock_pll =
		(freq <= MCP25XXFD_AUTO_PLL_MAX_CLOCK_FREQUENCY);

	/* decide on the effective clock rate */
	priv->clock_freq = freq;
	if (priv->config.clock_pll)
		priv->clock_freq *= MCP25XXFD_PLL_MULTIPLIER;
	if (priv->config.clock_div2)
		priv->clock_freq /= MCP25XXFD_SCLK_DIVIDER;

	/* calculate the clock frequencies to use
	 *
	 * setup clock speed is at most 1/4 the input clock speed
	 * the reason for the factor of 4 is that there is
	 * a clock divider in the controller that MAY be enabled in some
	 * circumstances so we may find a controller with that enabled
	 * during probe phase
	 */
	priv->spi_setup_speed_hz = freq / 4;

	/* normal operation clock speeds */
	priv->spi_normal_speed_hz = priv->clock_freq / 2;
	if (priv->config.clock_div2) {
		priv->spi_setup_speed_hz /= MCP25XXFD_SCLK_DIVIDER;
		priv->spi_normal_speed_hz /= MCP25XXFD_SCLK_DIVIDER;
	}

	/* set limit on speed */
	if (spi->max_speed_hz) {
		priv->spi_setup_speed_hz = min_t(int,
						 priv->spi_setup_speed_hz,
						 spi->max_speed_hz);
		priv->spi_normal_speed_hz = min_t(int,
						  priv->spi_normal_speed_hz,
						  spi->max_speed_hz);
	}

	/* use setup speed by default
	 * - this is switched when clock is enabled/disabled
	 */
	priv->spi_use_speed_hz = priv->spi_setup_speed_hz;

	return 0;
}

void mcp25xxfd_clock_fake_sleep(struct mcp25xxfd_priv *priv)
{
	priv->regs.osc &= ~(MCP25XXFD_OSC_OSCRDY |
			    MCP25XXFD_OSC_PLLRDY |
			    MCP25XXFD_OSC_SCLKRDY);
	priv->regs.osc |= MCP25XXFD_OSC_OSCDIS;
}
