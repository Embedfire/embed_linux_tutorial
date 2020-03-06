#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

extern struct bus_type xbus;

void xdev_release(struct device *dev)
{
	printk("%s-%s\n", __FILE__, __func__);
}

unsigned long id = 0;
ssize_t xdev_id_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	return sprintf(buf, "%d\n", id);
}

ssize_t xdev_id_store(struct device * dev, struct device_attribute * attr,
		      const char *buf, size_t count)
{
	kstrtoul(buf, 10, &id);
	return count;
}

DEVICE_ATTR(xdev_id, S_IRUSR | S_IWUSR, xdev_id_show, xdev_id_store);

static struct device xdev = {
	.init_name = "xdev",
	.bus = &xbus,
	.release = xdev_release,
};

static __init int xdev_init(void)
{
	printk("xdev init\n");
	device_register(&xdev);
	device_create_file(&xdev, &dev_attr_xdev_id);
	return 0;
}

module_init(xdev_init);

static __exit void xdev_exit(void)
{
	printk("xdev exit\n");
	device_remove_file(&xdev, &dev_attr_xdev_id);
	device_unregister(&xdev);
}

module_exit(xdev_exit);

MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");
