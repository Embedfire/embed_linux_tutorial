#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/leds_pwm.h>
#include <linux/slab.h>

#include "pwm_sub_system.h"


struct pwm_device	*red_led_pwm;  //定义pwm设备结构体
/*精简版 prob函数*/
static int led_pwm_probe_new(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *child; // 保存子节点
	struct device *dev = &pdev->dev;
	printk("match success \n");

	child = of_get_next_child(dev->of_node, NULL);
	if (child)
	{
		red_led_pwm = devm_of_pwm_get(dev, child, NULL);
		if (IS_ERR(red_led_pwm)) 
		{
			printk(KERN_ERR" red_led_pwm,get pwm  error!!\n");
			return -1;
		}
	}
	else
	{
		printk(KERN_ERR" red_led_pwm of_get_next_child  error!!\n");
		return -1;
	}

	/*配置频率100KHz 占空比80%*/
	pwm_config(red_led_pwm, 1000, 5000);
	/*反相 频率100KHz 占空比20%*/
	pwm_set_polarity(red_led_pwm, PWM_POLARITY_INVERSED);
	pwm_enable(red_led_pwm);

	return ret;
}

static int led_pwm_remove(struct platform_device *pdev)
{
	pwm_config(red_led_pwm, 0, 5000);
	return 0;
}

static const struct of_device_id of_pwm_leds_match[] = {
	{.compatible = "red_led_pwm"},
	{},
};

static struct platform_driver led_pwm_driver = {
	.probe		= led_pwm_probe_new,
	.remove		= led_pwm_remove,
	.driver		= {
		.name	= "led_pwm",
		.of_match_table = of_pwm_leds_match,
	},
};


/*
*驱动初始化函数
*/
static int __init pwm_leds_platform_driver_init(void)
{
	return platform_driver_register(&led_pwm_driver);
}

/*
*驱动注销函数
*/
static void __exit pwm_leds_platform_driver_exit(void)
{
	printk(KERN_ERR " pwm_leds_exit\n");
	/*注销平台设备*/
	platform_driver_unregister(&led_pwm_driver);
}

module_init(pwm_leds_platform_driver_init);
module_exit(pwm_leds_platform_driver_exit);

MODULE_LICENSE("GPL");


 





