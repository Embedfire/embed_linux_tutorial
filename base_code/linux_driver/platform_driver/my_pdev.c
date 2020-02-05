#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static int my_pdev_id = 0x1D;

static struct resource my_pdev_res[] = {
	[0] = {
	       .name = "mem",
	       .start = 0x1000,
	       .end = 0x2000,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .name = "irq",
	       .start = 0x1,
	       .end = 0x1,
	       .flags = IORESOURCE_IRQ,
	       },
};

static void my_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device my_pdev = {
	.id = 0,
	.name = "my_pdev",
	.resource = my_pdev_res,
	.num_resources = ARRAY_SIZE(my_pdev_res),
	.dev = {
		.platform_data = &my_pdev_id,
		.release = my_pdev_release,
		},
};

static __init int my_pdev_init(void)
{
	printk("my_pdev module loaded\n");

	platform_device_register(&my_pdev);

	return 0;
}

module_init(my_pdev_init);

static __exit void my_pdev_exit(void)
{
	printk("my_pdev module unloaded\n");

	platform_device_unregister(&my_pdev);
}

module_exit(my_pdev_exit);

MODULE_AUTHOR("Embedfire");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("the example for platform device");
