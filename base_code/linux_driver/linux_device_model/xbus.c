#include <linux/init.h>
#include <linux/module.h>

#include <linux/device.h>

int xbus_match(struct device *dev, struct device_driver *drv)
{

	printk("%s-%s\n",__FILE__, __func__);
	if(!strncmp(dev_name(dev), drv->name, strlen(drv->name))){
		printk("dev & drv match\n");
		return 1;	
	}
	return 0;

}

static char *bus_name = "xbus";

ssize_t xbus_test_show(struct bus_type *bus, char *buf)
{
	return sprintf(buf, "%s\n", bus_name);
}

BUS_ATTR(xbus_test, S_IRUSR, xbus_test_show, NULL);

static struct bus_type xbus = {
	.name = "xbus",
	.match = xbus_match,
};

EXPORT_SYMBOL(xbus);

static __init int xbus_init(void)
{
	printk("xbus init\n");
	
	bus_register(&xbus);
	bus_create_file(&xbus, &bus_attr_xbus_test);
	return 0;
}
module_init(xbus_init);


static __exit void xbus_exit(void)
{
	printk("xbus exit\n");
	bus_remove_file(&xbus, &bus_attr_xbus_test);
	bus_unregister(&xbus);
}
module_exit(xbus_exit);

MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");


