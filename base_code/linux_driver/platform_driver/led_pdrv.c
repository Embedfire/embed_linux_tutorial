#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define DEV_MAJOR 243
#define DEV_NAME  "led"

static struct class *my_led_class;

struct led_data {
	unsigned int led_pin;
	unsigned int clk_regshift;

	unsigned int __iomem *va_dr;
	unsigned int __iomem *va_gdir;
	unsigned int __iomem *va_iomuxc_mux;
	unsigned int __iomem *va_ccm_ccgrx;
	unsigned int __iomem *va_iomux_pad;	

	struct cdev led_cdev;

};

static int led_cdev_open(struct inode *inode, struct file *filp)
{
	printk("%s\n", __func__);
	
	struct led_data *cur_led = container_of(inode->i_cdev, struct led_data, led_cdev);
	unsigned int val = 0;

	val = readl(cur_led->va_ccm_ccgrx);
	val &= ~(3 << cur_led->clk_regshift);
	val |= (3 << cur_led->clk_regshift);
	writel(val, cur_led->va_ccm_ccgrx);

	writel(5, cur_led->va_iomuxc_mux);

	writel(0x1F838, cur_led->va_iomux_pad);

	val = readl(cur_led->va_gdir);
	val &= ~(1 << cur_led->led_pin);
	val |= (1 << cur_led->led_pin);
	writel(val, cur_led->va_gdir);

	val = readl(cur_led->va_dr);
	val |= (0x01 << cur_led->led_pin);
	writel(val, cur_led->va_dr);

	filp->private_data = cur_led;

	return 0;
}


static int led_cdev_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t led_cdev_write(struct file *filp, const char __user * buf,
			      size_t count, loff_t * ppos)
{
	unsigned long val = 0;
	unsigned long ret = 0;

	int tmp = count;

	struct led_data *cur_led = (struct led_data *)filp->private_data;

	kstrtoul_from_user(buf, tmp, 10, &ret);

	val = readl(cur_led->va_dr);
	if (ret == 0)
		val &= ~(0x01 << cur_led->led_pin);
	else
		val |= (0x01 << cur_led->led_pin);

	writel(val, cur_led->va_dr);
	*ppos += tmp;

	return tmp;
}

static struct file_operations led_cdev_fops = {
	.open = led_cdev_open,
	.release = led_cdev_release,
	.write = led_cdev_write,
};



static int led_pdrv_probe(struct platform_device *pdev)
{
	struct led_data *cur_led;
	unsigned int *led_hwinfo;
	
	struct resource *mem_dr;
	struct resource *mem_gdir;
	struct resource *mem_iomuxc_mux;
	struct resource *mem_ccm_ccgrx;
	struct resource *mem_iomux_pad; 	

	dev_t cur_dev;

	int ret = 0;
	
	printk("led platform driver probe\n");

	cur_led = devm_kzalloc(&pdev->dev, sizeof(struct led_data), GFP_KERNEL);
	if(!cur_led)
		return -ENOMEM;
	led_hwinfo = devm_kzalloc(&pdev->dev, sizeof(unsigned int)*2, GFP_KERNEL);
	if(!led_hwinfo)
		return -ENOMEM;

	/* get the pin for led and the reg's shift */
	led_hwinfo = dev_get_platdata(&pdev->dev);

	cur_led->led_pin = led_hwinfo[0];
	cur_led->clk_regshift = led_hwinfo[1];
	/* get platform resource */
	mem_dr = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mem_gdir = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	mem_iomuxc_mux = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	mem_ccm_ccgrx = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	mem_iomux_pad = platform_get_resource(pdev, IORESOURCE_MEM, 4);

	cur_led->va_dr =
	    devm_ioremap(&pdev->dev, mem_dr->start, resource_size(mem_dr));
	cur_led->va_gdir =
	    devm_ioremap(&pdev->dev, mem_gdir->start, resource_size(mem_gdir));
	cur_led->va_iomuxc_mux =
	    devm_ioremap(&pdev->dev, mem_iomuxc_mux->start,
			 resource_size(mem_iomuxc_mux));
	cur_led->va_ccm_ccgrx =
	    devm_ioremap(&pdev->dev, mem_ccm_ccgrx->start,
			 resource_size(mem_ccm_ccgrx));
	cur_led->va_iomux_pad =
	    devm_ioremap(&pdev->dev, mem_iomux_pad->start,
			 resource_size(mem_iomux_pad));

	cur_dev = MKDEV(DEV_MAJOR, pdev->id);

	register_chrdev_region(cur_dev, 1, "led_cdev");

	cdev_init(&cur_led->led_cdev, &led_cdev_fops);

	ret = cdev_add(&cur_led->led_cdev, cur_dev, 1);
	if(ret < 0)
	{
		printk("fail to add cdev\n");
		goto add_err;
	}
	
	device_create(my_led_class, NULL, cur_dev, NULL, DEV_NAME "%d", pdev->id);

	/* save as drvdata */ 
	platform_set_drvdata(pdev, cur_led);

	return 0;

add_err:
	unregister_chrdev_region(cur_dev, 1);
	return ret;
}


static int led_pdrv_remove(struct platform_device *pdev)
{
	dev_t cur_dev; 
	struct led_data *cur_data = platform_get_drvdata(pdev);


	printk("led platform driver remove\n");

	cur_dev = MKDEV(DEV_MAJOR, pdev->id);

	cdev_del(&cur_data->led_cdev);

	device_destroy(my_led_class, cur_dev);

	unregister_chrdev_region(cur_dev, 1);

	return 0;
}

static struct platform_device_id led_pdev_ids[] = {
	{.name = "led_pdev"},
	{}
};

MODULE_DEVICE_TABLE(platform, led_pdev_ids);

static struct platform_driver led_pdrv = {
	
	.probe = led_pdrv_probe,
	.remove = led_pdrv_remove,
	.driver.name = "led_pdev",
	.id_table = led_pdev_ids,
};



static __init int led_pdrv_init(void)
{
	printk("led platform driver init\n");

	my_led_class = class_create(THIS_MODULE, "my_leds");
	
	platform_driver_register(&led_pdrv);

	return 0;
}
module_init(led_pdrv_init);


static __exit void led_pdrv_exit(void)
{
	printk("led platform driver exit\n");	

	platform_driver_unregister(&led_pdrv);

	class_destroy(my_led_class);

}
module_exit(led_pdrv_exit);

MODULE_AUTHOR("Embedfire");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("the example for platform driver");
