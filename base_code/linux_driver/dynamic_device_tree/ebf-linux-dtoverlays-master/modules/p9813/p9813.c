/*
 * Chainable RGB LED (P9813) Driver
 *
 * Copyright (C) 2019 Seeed Studio
 * Peter Yang <turmary@126.com>
 *
 * Released under the GPLv2
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>

#define DEV_NAME "p981x"

/* FCLK <= 15MHz */
#define P981X_PULSE_DLY		50    /* ns */

#define P981X_LINE_LEN		64   /* chars */

/*
 * This driver creates a character device (/dev/p981xN) which exposes the
 * Chainable RGB LED (P9813)
 */

struct p981x_dev {
	/* misc device descriptor */
	struct miscdevice miscdev;

	/* pins */
	int     pin_clk;
	int     pin_data;

	/* locks */
	struct mutex mutex;

	/* LEDs count */
	int	count;
	/* LEDs data buffer */
	u32*    buffer;

	/* command line index */
	int     index;
	/* command line data */
	char    line[P981X_LINE_LEN + 1];
};
#define to_p981x_dev(dev)  container_of((dev), struct p981x_dev, miscdev)
static int dev_index = 0;


static int p981x_send_byte(struct p981x_dev *ppd, u8 b) {
	int i;

	for (i = 0; i < 8; i++) {
		/* DATA LEVEL */
		gpio_set_value(ppd->pin_data, !!(b & 0x80UL));

		/* ONE PULSE CLOCK */
		gpio_set_value(ppd->pin_clk, 0);
		ndelay(P981X_PULSE_DLY);
		gpio_set_value(ppd->pin_clk, 1);
		ndelay(P981X_PULSE_DLY);
		b <<= 1;
	}
	return 0;
}

static int p981x_send_u32(struct p981x_dev *ppd, u32 word) {
	int i;

	for (i = 3; i >= 0; i--) {
		p981x_send_byte(ppd, ((u8*)&word)[i]);
	}
	return 0;
}

static int p981x_send_frame(struct p981x_dev *ppd) {
	int i;

	/* Send data frame prefix (32x "0") */
	p981x_send_u32(ppd, 0x0UL);

	/* Send color data for each one of the leds */
	for (i = 0; i < ppd->count && ppd->buffer; i++) {
		p981x_send_u32(ppd, ppd->buffer[i]);
	}

	/* Terminate data frame (32x "0") */
	p981x_send_u32(ppd, 0x0UL);
	return 0;
}

static u32 p981x_color(u8 red, u8 green, u8 blue) {
	u32 color;

	color  = 0xC0000000UL;
	color |= (~blue  & 0xC0UL) << 22;
	color |= (~green & 0xC0UL) << 20;
	color |= (~red   & 0xC0UL) << 18;
	color |= blue  << 16;
	color |= green << 8;
	color |= red   << 0;
	return color;
}

static int p981x_cmds(struct p981x_dev *ppd) {
	char* endp;
	char* cp;
	int i, rc;
	#define COLOR_CNT	4
	int color[COLOR_CNT];

	switch (ppd->line[0]) {
	case 'N':
		rc = sscanf(&ppd->line[1], "%d", &ppd->count);
		if (ppd->buffer) {
			kfree(ppd->buffer);
			ppd->buffer = NULL;
		}
		if (ppd->count > 0) {
			ppd->buffer = kmalloc(sizeof(u32) * ppd->count, GFP_KERNEL);
		}
		break;

	case 'D':
		endp = &ppd->line[1];
		for (i = 0; i < COLOR_CNT && (cp = strsep(&endp, " \t")) != NULL; ) {
			long long ll;

			/* format: led-index red green blue */
			if (! *cp) {
				continue;
			}
			rc = kstrtoll(cp, 0, &ll);
			if (rc < 0) {
				break;
			}
			color[i++] = ll;

			pr_debug("color%d = 0x%02X (%d)\n", i - 1, (u8)ll, (u8)ll);
		}
		if (i >= COLOR_CNT) {
			rc = color[0];
			if (0 <= rc && rc < ppd->count && ppd->buffer) {
				ppd->buffer[rc] =
					p981x_color(color[1], color[2], color[3]);
			}
		}
		break;
	}

	ppd->index = 0;
	return 0;
}

static ssize_t p981x_write(struct file *f, const char __user *buf,
			     size_t len, loff_t *f_pos)
{
	struct p981x_dev *ppd = to_p981x_dev(f->private_data);
	size_t i;
	char ch;

	for (i = 0; i < len; i++) {
		if (copy_from_user(&ch, buf + i, 1)) {
			return -EFAULT;
		}

		switch (ch) {
		case '\r':
		case '\n':
		case '\0':
			ch = '\0';
			break;
		default:
			break;
		}
		if ( ! ppd->index && (ch == ' ' || ch == '\t')) {
			/* skip prefix space */
			continue;
		}
		ppd->line[ppd->index++] = ch;
		if (ppd->index >= P981X_LINE_LEN || !ch) {
			p981x_cmds(ppd);
		}
	}

	/* Update display */
	p981x_send_frame(ppd);

	return len;
}

static int p981x_open(struct inode *inode, struct file *f)
{
	struct p981x_dev *ppd = to_p981x_dev(f->private_data);
	/* struct device* dev = ppd->miscdev.this_device; */

	if (!mutex_trylock(&ppd->mutex)) {
		pr_debug("Device Busy\n");
		return -EBUSY;
	}
	return 0;
}

static int p981x_release(struct inode *inode, struct file *f)
{
	struct p981x_dev *ppd = to_p981x_dev(f->private_data);

	mutex_unlock(&ppd->mutex);
	return 0;
}

static const struct file_operations p981x_fops = {
	.owner		= THIS_MODULE,
	.write		= p981x_write,
	.open		= p981x_open,
	.release	= p981x_release
};

static const struct miscdevice misc_def = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DEV_NAME,
	.mode		= S_IWUGO,
	.fops		= &p981x_fops
};

/* Initialise GPIO */
static int p981x_gpio_init(int gpio)
{
	/* check that GPIO is valid */
	if ( !gpio_is_valid(gpio) ){
		pr_err("GPIO %d is not valid\n", gpio );
		return -EINVAL;
	}

	/* request the GPIO pin */
	if ( gpio_request(gpio, DEV_NAME) != 0 ) {
		pr_err("Unable to request GPIO %d\n", gpio );
		return -EINVAL;
	}

	/* make GPIO an output */
	if ( gpio_direction_output(gpio, 0) != 0 ) {
		pr_err("Failed to make GPIO %d as output\n", gpio );
		return -EINVAL;
	}
	return 0;
}

static int p981x_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev;
	struct p981x_dev *ppd;
	int rc;

	/* Allocate memory for our private structure */
	ppd = kzalloc(sizeof(*ppd), GFP_KERNEL);
	if (!ppd) {
		return -ENOMEM;
	}

	if (np) {
		/*
		 * first  gpio: clock pin
		 * second gpio: data  pin
		 */
		ppd->pin_clk  = of_get_gpio(np, 0);
		ppd->pin_data = of_get_gpio(np, 1);
	} else {
		ppd->pin_clk = ppd->pin_data = -1;
	}

	if (ppd->pin_clk < 0 || ppd->pin_data < 0) {
		pr_err("Failed to get clock & data gpio in device tree\n");
		rc = -EINVAL;
		goto free_p981x;
	}
	if (p981x_gpio_init(ppd->pin_clk) || p981x_gpio_init(ppd->pin_data)) {
		rc = -EINVAL;
		goto free_p981x;
	}

	mutex_init(&ppd->mutex);

	ppd->miscdev = misc_def;
	ppd->miscdev.name = kasprintf(GFP_KERNEL, "%s%d",
				misc_def.name, dev_index++);
	rc = misc_register(&ppd->miscdev);
	if (rc) {
		pr_err("Failed to register as misc device %s\n", misc_def.name);
		goto free_p981x;
	}

	/* platform device drvdata */
	dev_set_drvdata(&pdev->dev, ppd);

	/* misc device drvdata */
	dev = ppd->miscdev.this_device;
	dev_set_drvdata(dev, ppd);

	pr_info("device %s registered, using clock:%d data:%d\n",
		ppd->miscdev.name, ppd->pin_clk, ppd->pin_data);
	return 0;

free_p981x:
	kfree(ppd);
	return rc;
}

static int p981x_remove(struct platform_device *pdev)
{
	struct p981x_dev *ppd = platform_get_drvdata(pdev);

	gpio_free(ppd->pin_clk);
	gpio_free(ppd->pin_data);

	if (ppd->buffer) {
		kfree(ppd->buffer);
		ppd->buffer = NULL;
	}

	misc_deregister(&ppd->miscdev);
	kfree(ppd->miscdev.name);
	kfree(ppd);
	return 0;
}

static const struct of_device_id p981x_match[] = {
	{ .compatible = "dms," DEV_NAME },
	{ },
};

static struct platform_driver p981x_driver = {
	.driver	= {
		.name		= DEV_NAME,
		.of_match_table	= p981x_match,
	},
	.probe	= p981x_probe,
	.remove	= p981x_remove,
};

module_platform_driver(p981x_driver);

MODULE_DEVICE_TABLE(of, p981x_match);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Chainable RGB LED (P9813) Driver");
MODULE_AUTHOR("Peter Yang <turmary@126.com>");
