#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static int index0 = 0;
static int index1 = 1;

static struct platform_device_id my_pdev_ids[] = {
	{.name = "my_pdev",.driver_data = &index0},
	{.name = "my_test",.driver_data = &index1},
	{}
};

MODULE_DEVICE_TABLE(platform, my_pdev_ids);

static char *name;
static int *index;

static int my_pdrv_probe(struct platform_device *pdev)
{
	struct resource *mem = NULL;
	int irq;
	struct platform_device_id *id_match = pdev->id_entry;
	int *pdev_id = NULL;
	name = id_match->name;
	index = id_match->driver_data;
	printk("Hello! %s probed!The index is : %d\n", name, *index);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		printk("Resource not available\n");
		return -1;
	}
	printk("The name : %s, The start : %d, The end : %d\n", mem->name,
	       mem->start, mem->end);
	irq = platform_get_irq(pdev, 0);
	printk("The irq : %d\n", irq);

	pdev_id = dev_get_platdata(&pdev->dev);
	printk("The device id : 0x%x\n", *pdev_id);
	return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
	printk("Hello! %s removed!The index is : %d\n", name, *index);
	return 0;
}

static struct platform_driver my_pdrv = {
	.probe = my_pdrv_probe,
	.remove = my_pdrv_remove,
	.driver = {
		   .name = "my_pdev",
		   .owner = THIS_MODULE,
		   },
	.id_table = my_pdev_ids,
};

static __init int my_pdrv_init(void)
{
	printk("my_pdrv module loaded\n");

	platform_driver_register(&my_pdrv);

	return 0;
}

module_init(my_pdrv_init);

static __exit void my_pdrv_exit(void)
{
	printk("my_pdrv module unloaded\n");

	platform_driver_unregister(&my_pdrv);

}

module_exit(my_pdrv_exit);
MODULE_AUTHOR("Embedfire");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("the example for platform driver");
