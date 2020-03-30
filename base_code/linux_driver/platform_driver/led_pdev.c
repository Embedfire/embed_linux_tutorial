#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define CCM_CCGR1 															0x20C406C	//时钟控制寄存器
#define IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04 				0x20E006C	//GPIO1_04复用功能选择寄存器
#define IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04 				0x20E02F8	//PAD属性设置寄存器
#define GPIO1_GDIR 															0x0209C004	//GPIO方向设置寄存器（输入或输出）
#define GPIO1_DR 																0x0209C000	//GPIO输出状态寄存器

#define CCM_CCGR3 															0x020C4074
#define GPIO4_GDIR 															0x020A8004
#define GPIO4_DR 																0x020A8000

#define IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO020 			0x020E01E0
#define IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO020 			0x020E046C

#define IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO019 			0x020E01DC
#define IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO019 			0x020E0468

static struct resource rled_resource[] = {
	[0] = DEFINE_RES_MEM(GPIO1_DR, 4),
	[1] = DEFINE_RES_MEM(GPIO1_GDIR, 4),
	[2] = DEFINE_RES_MEM(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04, 4),
	[3] = DEFINE_RES_MEM(CCM_CCGR1, 4),
	[4] = DEFINE_RES_MEM(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04, 4),
};

static struct resource gled_resource[] = {
	[0] = DEFINE_RES_MEM(GPIO4_DR, 4),
	[1] = DEFINE_RES_MEM(GPIO4_GDIR, 4),
	[2] = DEFINE_RES_MEM(IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO020, 4),
	[3] = DEFINE_RES_MEM(CCM_CCGR3, 4),
	[4] = DEFINE_RES_MEM(IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO020, 4),
};

static struct resource bled_resource[] = {
	[0] = DEFINE_RES_MEM(GPIO4_DR, 4),
	[1] = DEFINE_RES_MEM(GPIO4_GDIR, 4),
	[2] = DEFINE_RES_MEM(IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO019, 4),
	[3] = DEFINE_RES_MEM(CCM_CCGR3, 4),
	[4] = DEFINE_RES_MEM(IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO019, 4),
};
/* not used */ 
static void led_release(struct device *dev)
{

}

/* led hardware information */
unsigned int rled_hwinfo[2] = { 4, 26 };
unsigned int gled_hwinfo[2] = { 20, 12 };
unsigned int bled_hwinfo[2] = { 19, 12 };

/* red led device */ 
static struct platform_device rled_pdev = {
	.name = "led_pdev",
	.id = 0,
	.num_resources = ARRAY_SIZE(rled_resource),
	.resource = rled_resource,
	.dev = {
		.release = led_release,
		.platform_data = rled_hwinfo,
		},
};
/* green led device */ 
static struct platform_device gled_pdev = {
	.name = "led_pdev",
	.id = 1,
	.num_resources = ARRAY_SIZE(gled_resource),
	.resource = gled_resource,
	.dev = {
		.release = led_release,
		.platform_data = gled_hwinfo,
		},
};
/* blue led device */ 
static struct platform_device bled_pdev = {
	.name = "led_pdev",
	.id = 2,
	.num_resources = ARRAY_SIZE(bled_resource),
	.resource = bled_resource,
	.dev = {
		.release = led_release,
		.platform_data = bled_hwinfo,
		},
};

static __init int led_pdev_init(void)
{
	printk("pdev init\n");
	platform_device_register(&rled_pdev);
	platform_device_register(&gled_pdev);
	platform_device_register(&bled_pdev);
	return 0;
}

module_init(led_pdev_init);

static __exit void led_pdev_exit(void)
{
	printk("pdev exit\n");
	platform_device_unregister(&rled_pdev);
	platform_device_unregister(&gled_pdev);
	platform_device_unregister(&bled_pdev);
}

module_exit(led_pdev_exit);

MODULE_AUTHOR("embedfire");
MODULE_LICENSE("GPL");
