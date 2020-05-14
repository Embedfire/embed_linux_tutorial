/*
 * hd44780-i2c.c
 *
 * Implements I2C interface for JHD1802/HD44780 LCD.
 * Driver is based on driver written by gorskima
 * (https://github.com/gorskima/hd44780-i2c).
 *
 * Port to support JHD1802 by Peter Yang <turmary@126.com>
 * Copyright (C) 2019 Seeed Studio
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>

#include "hd44780.h"

#define CLASS_NAME	"hd44780"
#define NAME		"hd44780"
#define NUM_DEVICES	8

static struct class *hd44780_class;
static dev_t dev_no;
/* We start with -1 so that first returned minor is 0 */
static atomic_t next_minor = ATOMIC_INIT(-1);

static LIST_HEAD(hd44780_list);
static DEFINE_SPINLOCK(hd44780_list_lock);

/* Device attributes */

static ssize_t geometry_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd;
	struct hd44780_geometry *geo;
	
	lcd = dev_get_drvdata(dev);
	geo = lcd->geometry;

	return scnprintf(buf, PAGE_SIZE, "%dx%d\n", geo->cols, geo->rows);
}

static ssize_t geometry_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd;
	struct hd44780_geometry *geo;
	int cols = 0, rows = 0, i;

	sscanf(buf, "%dx%d", &cols, &rows);

	for (i = 0; hd44780_geometries[i] != NULL; i++) {
		geo = hd44780_geometries[i];

		if (geo->cols == cols && geo->rows == rows) {
			lcd = dev_get_drvdata(dev);

			mutex_lock(&lcd->lock);
			hd44780_set_geometry(lcd, geo);
			mutex_unlock(&lcd->lock);

			break;
		}
	}

	return count;
}
static DEVICE_ATTR_RW(geometry);

static ssize_t backlight_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", lcd->backlight);
}

static ssize_t backlight_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	mutex_lock(&lcd->lock);
	hd44780_set_backlight(lcd, buf[0] == '1');
	mutex_unlock(&lcd->lock);

	return count;
}
static DEVICE_ATTR_RW(backlight);

static ssize_t cursor_blink_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", lcd->cursor_blink);
}

static ssize_t cursor_blink_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	mutex_lock(&lcd->lock);
	hd44780_set_cursor_blink(lcd, buf[0] == '1');
	mutex_unlock(&lcd->lock);

	return count;
}
static DEVICE_ATTR_RW(cursor_blink);

static ssize_t cursor_display_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", lcd->cursor_display);
}

static ssize_t cursor_display_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct hd44780 *lcd = dev_get_drvdata(dev);

	mutex_lock(&lcd->lock);
	hd44780_set_cursor_display(lcd, buf[0] == '1');
	mutex_unlock(&lcd->lock);

	return count;
}
static DEVICE_ATTR_RW(cursor_display);

static struct attribute *hd44780_device_attrs[] = {
	&dev_attr_geometry.attr,
	&dev_attr_backlight.attr,
	&dev_attr_cursor_blink.attr,
	&dev_attr_cursor_display.attr,
	NULL
};
ATTRIBUTE_GROUPS(hd44780_device);

/* File operations */

static int hd44780_file_open(struct inode *inode, struct file *filp)
{
	filp->private_data = container_of(inode->i_cdev, struct hd44780, cdev);
	return 0;
}

static int hd44780_file_release(struct inode *inode, struct file *filp)
{
	struct hd44780 *lcd = filp->private_data;
	hd44780_flush(lcd);
	return 0;
}

static ssize_t hd44780_file_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	struct hd44780 *lcd;
	size_t n;

	lcd = filp->private_data;
	n = min(count, (size_t)BUF_SIZE);

	// TODO: Consider using an interruptible lock
	mutex_lock(&lcd->lock);

	// TODO: Support partial writes during errors?
	if (copy_from_user(lcd->buf, buf, n)) {
		mutex_unlock(&lcd->lock);
		return -EFAULT;
	}

	hd44780_write(lcd, lcd->buf, n);

	mutex_unlock(&lcd->lock);

	return n;
}

static void hd44780_init(struct hd44780 *lcd, struct hd44780_geometry *geometry,
		struct i2c_client *i2c_client)
{
	lcd->geometry = geometry;
	lcd->i2c_client = i2c_client;
	lcd->pos.row = 0;
	lcd->pos.col = 0;
	memset(lcd->esc_seq_buf.buf, 0, ESC_SEQ_BUF_SIZE);
	lcd->esc_seq_buf.length = 0;
	lcd->is_in_esc_seq = false;
	lcd->backlight = true;
	lcd->cursor_blink = true;
	lcd->cursor_display = true;
	mutex_init(&lcd->lock);
}

static struct file_operations fops = {
	.open = hd44780_file_open,
	.release = hd44780_file_release,
	.write = hd44780_file_write,
};

static int hd44780_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	dev_t devt;
	struct hd44780 *lcd;
	struct device *device;
	int ret, minor;

	minor = atomic_inc_return(&next_minor);
	devt = MKDEV(MAJOR(dev_no), minor);

	lcd = (struct hd44780 *)kmalloc(sizeof(struct hd44780), GFP_KERNEL);
	if (!lcd) {
		return -ENOMEM;
	}

	/* JHD1802, default resolution 16x2 */
	hd44780_init(lcd, hd44780_geometries[1], client);

	spin_lock(&hd44780_list_lock);
	list_add(&lcd->list, &hd44780_list);
	spin_unlock(&hd44780_list_lock);

	cdev_init(&lcd->cdev, &fops);
	ret = cdev_add(&lcd->cdev, devt, 1);
	if (ret) {
		pr_warn("Can't add cdev\n");
		goto exit;
	}

	device = device_create_with_groups(hd44780_class, NULL, devt, NULL,
		hd44780_device_groups, "lcd%d", MINOR(devt));

	if (IS_ERR(device)) {
		ret = PTR_ERR(device);
		pr_warn("Can't create device\n");
		goto del_exit;
	}

	dev_set_drvdata(device, lcd);
	lcd->device = device;

	hd44780_init_lcd(lcd);

	hd44780_print(lcd, "Grove-16x2 LCD\nJHD1802M0");
	lcd->dirty = true;
	
	return 0;

del_exit:
	cdev_del(&lcd->cdev);

	spin_lock(&hd44780_list_lock);
	list_del(&lcd->list);
	spin_unlock(&hd44780_list_lock);
exit:
	kfree(lcd);

	return ret;
}

static struct hd44780 * get_hd44780_by_i2c_client(struct i2c_client *i2c_client)
{
	struct hd44780 *lcd;

	spin_lock(&hd44780_list_lock);
	list_for_each_entry(lcd, &hd44780_list, list) {
		if (lcd->i2c_client->addr == i2c_client->addr) {
			spin_unlock(&hd44780_list_lock);
			return lcd;
		}
	}
	spin_unlock(&hd44780_list_lock);

	return NULL;
}


static int hd44780_remove(struct i2c_client *client)
{
	struct hd44780 *lcd;
	lcd = get_hd44780_by_i2c_client(client);
	device_destroy(hd44780_class, lcd->device->devt);
	cdev_del(&lcd->cdev);

	spin_lock(&hd44780_list_lock);
	list_del(&lcd->list);
	spin_unlock(&hd44780_list_lock);

	kfree(lcd);
	
	return 0;
}

static const struct i2c_device_id hd44780_id[] = {
	{ NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, hd44780_id);

static struct i2c_driver hd44780_driver = {
	.driver = {
		.name	= NAME,
		.owner	= THIS_MODULE,
	},
	.probe = hd44780_probe,
	.remove = hd44780_remove,
	.id_table = hd44780_id,
};

static int __init hd44780_mod_init(void)
{
	int ret;

	ret = alloc_chrdev_region(&dev_no, 0, NUM_DEVICES, NAME);
	if (ret) {
		pr_warn("Can't allocate chardev region");
		return ret;
	}

	hd44780_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(hd44780_class)) {
		ret = PTR_ERR(hd44780_class);
		pr_warn("Can't create %s class\n", CLASS_NAME);
		goto exit;
	}

	ret = i2c_add_driver(&hd44780_driver);
	if (ret) {
		pr_warn("Can't register I2C driver %s\n", hd44780_driver.driver.name);
		goto destroy_exit;
	}

	return 0;

destroy_exit:
	class_destroy(hd44780_class);
exit:
	unregister_chrdev_region(dev_no, NUM_DEVICES);

	return ret;
}
module_init(hd44780_mod_init);

static void __exit hd44780_mod_exit(void)
{
	i2c_del_driver(&hd44780_driver);
	class_destroy(hd44780_class);
	unregister_chrdev_region(dev_no, NUM_DEVICES);
}
module_exit(hd44780_mod_exit);

MODULE_AUTHOR("Mariusz Gorski <marius.gorski@gmail.com>");
MODULE_AUTHOR("Peter Yang <turmary@126.com>");
MODULE_DESCRIPTION("JHD1802 HD44780 I2C driver");
MODULE_LICENSE("GPL");
