#include <linux/init.h>
#include <linux/module.h>

#include <linux/device.h>

/************************************************************************
	* 函数负责总线下的设备以及驱动匹配
	* 使用字符串比较的方式，通过对比驱动以及设备的名字来确定是否匹配，
	* 如果相同， 则说明匹配成功，返回1；反之，则返回0
	***********************************************************************/
int xbus_match(struct device *dev, struct device_driver *drv)
{
	printk("%s-%s\n", __FILE__, __func__);
	if (!strncmp(dev_name(dev), drv->name, strlen(drv->name))) {
		printk("dev & drv match\n");
		return 1;
	}
	return 0;

}

//定义了一个bus_name变量，存放了该总线的名字
static char *bus_name = "xbus";
//提供show回调函数，这样用户便可以通过cat命令， 来查询总线的名称
ssize_t xbus_test_show(struct bus_type *bus, char *buf)
{
	return sprintf(buf, "%s\n", bus_name);
}
//设置该文件的文件权限为文件拥有者可读，组内成员以及其他成员不可操作
BUS_ATTR(xbus_test, S_IRUSR, xbus_test_show, NULL);

//定义了一种新的总线，名为xbus，总线结构体中最重要的一个成员，便是match回调函数
static struct bus_type xbus = {
	.name = "xbus",
	.match = xbus_match,
};

EXPORT_SYMBOL(xbus);

//注册总线
static __init int xbus_init(void)
{
	printk("xbus init\n");

	bus_register(&xbus);
	bus_create_file(&xbus, &bus_attr_xbus_test);
	return 0;
}

module_init(xbus_init);

//注销总线
static __exit void xbus_exit(void)
{
	printk("xbus exit\n");
	bus_remove_file(&xbus, &bus_attr_xbus_test);
	bus_unregister(&xbus);
}

module_exit(xbus_exit);

MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");
