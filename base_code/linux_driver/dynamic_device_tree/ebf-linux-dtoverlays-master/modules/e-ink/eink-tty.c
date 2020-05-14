/*
 * Triple Color E-Ink Display Driver
 *
 * Copyright (C) 2019 Seeed Studio
 * Peter Yang <turmary@126.com>
 *
 * Released under the GPL
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
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/termios.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/ioctls.h>
#include <linux/serial.h>
#include <linux/poll.h>

#define DEFAULT_BAUDRATE	230400
#define DEFAULT_TTY		"/dev/ttyS4"
#define DEFAULT_LINE_LEN	152
#define DEFAULT_LINES		152

#define EINK_HDR		'a'
#define EINK_HDR_RSP		'b'

#define EINK_CHK_PROGRESS	0
#define EINK_TAG_NEXT		"NEXT!"
#define EINK_TAG_DONE		"DONE!"
#define EINK_TAG_DBG		0
#define EINK_TIMEOUT		3000 /* ms */

#define EINK_LINE_DLY		48   /* ms */

#define EINK_POLL_WAIT		0

/*
 * This driver creates a character device (/dev/eink) which exposes the
 * Triple Color E-ink Display
 */

struct eink_dev {
	/* misc device descriptor */
	struct miscdevice miscdev;

	/* port (/dev/ttyS1 eg.) & speed (115200 eg.)*/
	const char* tty_port;
	u32	baudrate;

	/* locks */
	struct mutex mutex;

	/* screen x-res & y-res */
	u32	line_len, num_lines;
	u32	size;

	/* tty port info */
	struct file* tty;

	/* bytes left to written of one display data image */
	int bytes_left;
};
#define to_eink_dev(dev)  container_of((dev), struct eink_dev, miscdev)
static int dev_index = 0;

static long eink_tty_ioctl(struct file *f, unsigned int op,
				 unsigned long param)
{
	if (f->f_op->unlocked_ioctl)
		return f->f_op->unlocked_ioctl(f, op, param);

	return -ENOTTY;
}

static int eink_tty_write(struct file *f, unsigned char *buf, int count)
{
	loff_t pos = 0;
	return kernel_write(f, buf, count, &pos);
}

#if EINK_POLL_WAIT
static void eink_tty_read_poll_wait(struct file *f, int timeout)
{
	struct poll_wqueues table;
	ktime_t start, now;

	start = ktime_get();
	poll_initwait(&table);
	while (1) {
		long elapsed;
		int mask;

		mask = f->f_op->poll(f, &table.pt);
		if (mask & (POLLRDNORM | POLLRDBAND | POLLIN |
			    POLLHUP | POLLERR)) {
			break;
		}
		now = ktime_get();
		elapsed = ktime_us_delta(now, start);
		if (elapsed > timeout)
			break;
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(((timeout - elapsed) * HZ) / 10000);
	}
	poll_freewait(&table);
}
#endif

static int eink_tty_read(struct file *f, int timeout)
{
	unsigned char ch;
	int rc;
	loff_t pos = 0;

	rc = -ETIMEDOUT;
	if (IS_ERR(f)) {
		return -EINVAL;
	}

	#if EINK_POLL_WAIT
	if (f->f_op->poll) {
		eink_tty_read_poll_wait(f, timeout);

		if (kernel_read(f, &ch, 1, &pos) == 1)
			rc = ch;
	}
	else
	#endif
	{
		/* Device does not support poll, busy wait */
		int retries = 0;

		while (1) {
			if (kernel_read(f, &ch, 1, &pos) == 1) {
				rc = ch;
				break;
			}

			if (++retries > timeout) {
				break;
			}
			usleep_range(800, 1200);
		}
	}
	return rc;
}

static int eink_tty_block(struct file *f) {
	mm_segment_t oldfs;
	int fl;

	oldfs = get_fs();

	/* Set fd block */
	fl = eink_tty_ioctl(f, F_GETFL, 0);
	if (fl == -1) {
		return fl;
	}
	fl &= ~O_NONBLOCK;

	set_fs(KERNEL_DS);
	eink_tty_ioctl(f, F_SETFL, (unsigned)fl);
	set_fs(oldfs);
	return 0;
}

static int eink_tty_setspeed(struct file *f, int speed)
{
	struct termios termios;
	struct serial_struct serial;
	mm_segment_t oldfs;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	/* Set speed */
	eink_tty_ioctl(f, TCGETS, (unsigned long)&termios);
	termios.c_iflag = 0;
	termios.c_oflag = 0;
	termios.c_lflag = 0;
	termios.c_cflag = CLOCAL | CS8 | CREAD;
	termios.c_cc[VMIN] = 1;
	termios.c_cc[VTIME] = 0;

	switch (speed) {
	case 2400:
		termios.c_cflag |= B2400;
		break;
	case 4800:
		termios.c_cflag |= B4800;
		break;
	case 9600:
		termios.c_cflag |= B9600;
		break;
	case 19200:
		termios.c_cflag |= B19200;
		break;
	case 38400:
		termios.c_cflag |= B38400;
		break;
	case 57600:
		termios.c_cflag |= B57600;
		break;
	case 115200:
		termios.c_cflag |= B115200;
		break;
	case 230400:
		termios.c_cflag |= B230400;
		break;
	default:
		termios.c_cflag |= B9600;
		break;
	}
	eink_tty_ioctl(f, TCSETS, (unsigned long)&termios);

	/* Set low latency */
	eink_tty_ioctl(f, TIOCGSERIAL, (unsigned long)&serial);
	serial.flags |= ASYNC_LOW_LATENCY;
	eink_tty_ioctl(f, TIOCSSERIAL, (unsigned long)&serial);

	/* Clear input buffer */
	eink_tty_ioctl(f, TCFLSH, TCIOFLUSH);

	set_fs(oldfs);

	return 0;
}

#if EINK_CHK_PROGRESS
#define LINE_BUF_SZ	20
#define TIMEOUT_MAX	100
static int eink_check_progress(struct eink_dev *edev) {
	static char buf[LINE_BUF_SZ];
	static int index;
	int rc;
	int timeout;

	memset(buf, '\0', sizeof buf);
	for (timeout = TIMEOUT_MAX; timeout > 0; ) {
		if ((rc = eink_tty_read(edev->tty, 1)) < 0) {
			usleep_range(1000, 1000);
			timeout--;
			continue;
		}

		#if EINK_TAG_DBG
		pr_info("R %c x%02X\n", rc, rc);
		#endif

		if (rc == '\r' || rc == '\n') {
			if (!strcmp(buf, EINK_TAG_NEXT) ||
			    !strcmp(buf, EINK_TAG_DONE)) {
				#if EINK_TAG_DBG
				pr_info("next or done\n");
				#endif
				break;
			}
			index = 0;
			memset(buf, '\0', sizeof buf);
		} else if (index < LINE_BUF_SZ - 1) {
			buf[index++] = rc;
			buf[index] = '\0';
		}
	}
	if (timeout <= 0) {
		return -ETIMEDOUT;
	}

	#if EINK_TAG_DBG
	pr_info("timeout = %d\n", timeout);
	#endif

	/* Firmware bug: this time to wait burning flash data */
	msleep(EINK_LINE_DLY + timeout - TIMEOUT_MAX);
	return 0;
}
#endif

static ssize_t eink_write(struct file *f, const char __user *buf,
			     size_t len, loff_t *f_pos)
{
	struct eink_dev *edev = to_eink_dev(f->private_data);
	/* 4 lines each time */
	int each_max = edev->line_len / 8 * 4;
	int rc = 0;
	int left;

	for (left = len; left > 0; ) {
		int line = min(left, each_max);

		if ((rc = eink_tty_write(edev->tty, (u8*)buf, line)) < 0) {
			break;
		}
		buf += rc;
		left -= rc;

		/* e-ink slow wrtting */
		#if EINK_CHK_PROGRESS
		eink_check_progress(edev);
		#else
		msleep(EINK_LINE_DLY);
		#endif
	}
	if (len != left) {
		rc = len - left;
		edev->bytes_left -= rc;
	}
	return rc;
}

/*
 * header & response protocol
 */
static int eink_send_header(struct file* tty) {
	int rc, timeout;
	u8 header = EINK_HDR;

	/* clear receiving buffer, in 1 seconds */
	timeout = 100;
	while ((rc = eink_tty_read(tty, 10)) >= 0) {
		pr_info("recv: 0x%02X\n", rc);
		if (timeout-- <= 0) {
			return -EBUSY;
		}
	}

	/* it's blocked from now on */
	eink_tty_block(tty);

	if ((rc = eink_tty_write(tty, &header, sizeof header)) < 0) {
		pr_info("write e-ink error = %d\n", rc);
		return -EINVAL;
	}

	if ((rc = eink_tty_read(tty, EINK_TIMEOUT)) < 0) {
		pr_info("read e-ink error = %d\n", rc);
		return rc;
	}

	if (rc != EINK_HDR_RSP) {
		pr_info("invalid response = 0x%02x\n", rc);
		return -ENODEV;
	}

	return 0;
}

static int eink_open(struct inode *inode, struct file *f)
{
	struct eink_dev *edev = to_eink_dev(f->private_data);
	struct device* dev = edev->miscdev.this_device;
	int rc = 0;

	if (!mutex_trylock(&edev->mutex)) {
		pr_debug("Device Busy\n");
		return -EBUSY;
	}

	edev->tty = filp_open(edev->tty_port, O_RDWR | O_NONBLOCK, 0);
	if (IS_ERR(edev->tty)) {
		rc = (int)PTR_ERR(edev->tty);
		dev_err(dev, "file %s open error = %d\n", edev->tty_port, rc);
		goto fail;
	}

	/* Set configuration to the tty port */
	rc = eink_tty_setspeed(edev->tty, edev->baudrate);
	if (rc) {
		goto fail;
	}

	/* header to probe e-ink hardware */
	rc = eink_send_header(edev->tty);
	if (rc) {
		goto fail;
	}

	edev->bytes_left = edev->size;
	return 0;

fail:
	mutex_unlock(&edev->mutex);
	return rc;
}

static int eink_release(struct inode *inode, struct file *f)
{
	struct eink_dev *edev = to_eink_dev(f->private_data);
	mutex_unlock(&edev->mutex);

	filp_close(edev->tty, NULL);

	if (edev->bytes_left > 0) {
		pr_info("incomplete e-ink display data be written\n");
		return -EINVAL;
	}
	return 0;
}

static const struct file_operations eink_fops = {
	.owner		= THIS_MODULE,
	.write		= eink_write,
	.open		= eink_open,
	.release	= eink_release
};

static const struct miscdevice misc_def = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= "eink",
	.mode		= S_IWUGO,
	.fops		= &eink_fops
};

static int eink_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev;
	struct eink_dev *edev;
	int rc;

	/* Allocate memory for our private structure */
	edev = kzalloc(sizeof(*edev), GFP_KERNEL);
	if (!edev) {
		return -ENOMEM;
	}

	rc = of_property_read_string(np, "tty-port", &edev->tty_port);
	if (rc) {
		edev->tty_port = DEFAULT_TTY;
		pr_err("tty-port property not found, use default %s\n",
			edev->tty_port);
	}

	rc = of_property_read_u32(np, "baudrate", &edev->baudrate);
	if (rc) {
		edev->baudrate = DEFAULT_BAUDRATE;
		pr_err("baudrate property not found, use default %u\n",
			edev->baudrate);
	}

	rc = of_property_read_u32(np, "lines-len", &edev->line_len);
	if (rc) {
		edev->line_len = DEFAULT_LINE_LEN;
		pr_err("lines-len property not found, use default %u\n",
			edev->line_len);
	}

	rc = of_property_read_u32(np, "num-lines", &edev->num_lines);
	if (rc) {
		edev->num_lines = DEFAULT_LINES;
		pr_err("num-lines property not found, use default %u\n",
			edev->num_lines);
	}

	mutex_init(&edev->mutex);

	/* 1 pixel occupy 2 bits */
	edev->size = edev->line_len * edev->num_lines / 4;

	pr_devel("E-ink of size %u bytes found with %u lines of length %u\n",
		edev->size, edev->num_lines, edev->line_len);

	edev->miscdev = misc_def;
	edev->miscdev.name = kasprintf(GFP_KERNEL, "%s%d",
				misc_def.name, dev_index++);
	rc = misc_register(&edev->miscdev);
	if (rc) {
		pr_err("Failed to register as misc device\n");
		goto free_eink;
	}

	/* platform device drvdata */
	dev_set_drvdata(&pdev->dev, edev);

	/* misc device drvdata */
	dev = edev->miscdev.this_device;
	dev_set_drvdata(dev, edev);

	return 0;

free_eink:
	kfree(edev);
	return rc;
}

static int eink_remove(struct platform_device *pdev)
{
	struct eink_dev *edev = platform_get_drvdata(pdev);
	
	misc_deregister(&edev->miscdev);
	kfree(edev->miscdev.name);
	kfree(edev);
	return 0;
}

static const struct of_device_id eink_match[] = {
	{ .compatible = "seeed,eink" },
	{ },
};

static struct platform_driver eink_driver = {
	.driver	= {
		.name		= "eink",
		.of_match_table	= eink_match,
	},
	.probe	= eink_probe,
	.remove	= eink_remove,
};

module_platform_driver(eink_driver);

MODULE_DEVICE_TABLE(of, eink_match);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Triple Color E-Ink Display Driver");
MODULE_AUTHOR("Peter Yang <turmary@126.com>");
