#include <linux/init.h>
#include <linux/module.h>

#include <linux/device.h>

extern struct bus_type xbus;

char *name = "xdrv";

//保证store和show函数的前缀与驱动属性文件一致，drvname_show()的前缀和drvname
ssize_t drvname_show(struct device_driver *drv, char *buf)
{
	return sprintf(buf, "%s\n", name);
}

//DRIVER_ATTR_RO定义了一个drvname属性文件
DRIVER_ATTR_RO(drvname);

int xdrv_probe(struct device *dev)
{
	printk("%s-%s\n", __FILE__, __func__);
	return 0;
}

int xdrv_remove(struct device *dev)
{
	printk("%s-%s\n", __FILE__, __func__);
	return 0;
}

//定义了一个驱动结构体xdrv，名字需要和设备的名字相同，否则就不能成功匹配
static struct device_driver xdrv = {
	.name = "xdev",
	//该驱动挂载在已经注册好的总线xbus下。
	.bus = &xbus,
	//当驱动和设备匹配成功之后，便会执行驱动的probe函数
	.probe = xdrv_probe,
	//当注销驱动时，需要关闭物理设备的某些功能等
	.remove = xdrv_remove,
};

//调用driver_register函数以及driver_create_file函数进行注册我们的驱动以及驱动属性文件
static __init int xdrv_init(void)
{
	printk("xdrv init\n");
	driver_register(&xdrv);
	driver_create_file(&xdrv, &driver_attr_drvname);
	return 0;
}

module_init(xdrv_init);

//注销驱动以及驱动属性文件
static __exit void xdrv_exit(void)
{
	printk("xdrv exit\n");
	driver_remove_file(&xdrv, &driver_attr_drvname);
	driver_unregister(&xdrv);
}

module_exit(xdrv_exit);

MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");
