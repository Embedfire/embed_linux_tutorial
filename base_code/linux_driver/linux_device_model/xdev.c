#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>

extern struct bus_type xbus;

void xdev_release(struct device *dev)
{
	printk("%s-%s\n", __FILE__, __func__);
}

unsigned long id = 0;

//show回调函数中，直接将id的值通过sprintf函数拷贝至buf中。
ssize_t xdev_id_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	return sprintf(buf, "%d\n", id);
}

/*********************************************************************************************
	* store回调函数则是利用kstrtoul函数，
	* 该函数有三个参数，其中第二个参数是采用几进制的方式， 这里我们传入的是10，意味着buf中的内容将转换为10进制的数传递给id，
	* 实现了通过sysfs修改驱动的目的。
	*********************************************************************************************/
ssize_t xdev_id_store(struct device * dev, struct device_attribute * attr,
		      const char *buf, size_t count)
{
	kstrtoul(buf, 10, &id);
	return count;
}

//DEVICE_ATTR宏定义定义了xdev_id，设置该文件的文件权限是文件拥有者可读可写，组内成员以及其他成员不可操作
DEVICE_ATTR(xdev_id, S_IRUSR | S_IWUSR, xdev_id_show, xdev_id_store);

static struct device xdev = {
	.init_name = "xdev",
	.bus = &xbus,
	.release = xdev_release,
};

//设备结构体以及属性文件结构体注册
static __init int xdev_init(void)
{
	printk("xdev init\n");
	device_register(&xdev);
	device_create_file(&xdev, &dev_attr_xdev_id);
	return 0;
}

module_init(xdev_init);

//设备结构体以及属性文件结构体注销。
static __exit void xdev_exit(void)
{
	printk("xdev exit\n");
	device_remove_file(&xdev, &dev_attr_xdev_id);
	device_unregister(&xdev);
}

module_exit(xdev_exit);

MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");
