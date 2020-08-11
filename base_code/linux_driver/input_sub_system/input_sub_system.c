
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/of_irq.h>

#include "input_sub_system.h"

#include <linux/input.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

struct device_node *button_device_node = NULL; //定义按键设备节点结构体
unsigned button_GPIO_number = 0;			   //保存button使用的GPIO引脚编号
u32 interrupt_number = 0;					   // button 引脚中断编号

/*input 子系统相关内容*/
#define BUTTON_NAME "button"
struct input_dev *button_input_dev = NULL; //定义按键对应的输入子系统结构体


/*按键中断处理函数*/
static irqreturn_t button_irq_hander(int irq, void *dev_id)
{
	int button_satus = 0;

	/*读取按键引脚的电平，根据读取得到的结果输入按键状态*/
	button_satus = gpio_get_value(button_GPIO_number);
	if(0 == button_satus)
	{
		input_report_key(button_input_dev, KEY_1, 0);
		input_sync(button_input_dev);
	}
	else
	{
		input_report_key(button_input_dev, KEY_1, 1);
		input_sync(button_input_dev);
	}
	
	return IRQ_HANDLED;
}


/*
*驱动初始化函数
*/
static int __init button_driver_init(void)
{

	int error;
	printk(KERN_ERR"button_driver_init \n");

	/*获取按键 设备树节点*/
	button_device_node = of_find_node_by_path("/button_interrupt");
	if (NULL == button_device_node)
	{
		printk(KERN_ERR "of_find_node_by_path error!");
		return -1;
	}

	/*获取按键使用的GPIO*/
	button_GPIO_number = of_get_named_gpio(button_device_node, "button_gpio", 0);
	if (0 == button_GPIO_number)
	{
		printk(KERN_ERR"of_get_named_gpio error");
		return -1;
	}

	/*申请GPIO  , 记得释放*/
	error = gpio_request(button_GPIO_number, "button_gpio");
	if (error < 0)
	{
		printk(KERN_ERR "gpio_request error");
		gpio_free(button_GPIO_number);
		return -1;
	}

	error = gpio_direction_input(button_GPIO_number); //设置引脚为输入模式

	/*获取中断号*/
	interrupt_number = irq_of_parse_and_map(button_device_node, 0);
	printk(KERN_ERR"\n interrupt_number =  %d \n", interrupt_number);

	/*申请中断, 记得释放*/
	error = request_irq(interrupt_number, button_irq_hander, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "button_interrupt", NULL);
	if (error != 0)
	{
		printk(KERN_ERR "request_irq error");
		gpio_free(button_GPIO_number);
		free_irq(interrupt_number, NULL);
		return -1;
	}

	/*申请输入子系统结构体*/
	button_input_dev = input_allocate_device();
	if (NULL == button_input_dev)
	{
		printk(KERN_ERR "input_allocate_device error");
		return -1;
	}
	button_input_dev->name = BUTTON_NAME;

	/*设置要使用的输入事件类型*/
	button_input_dev->evbit[0] = BIT_MASK(EV_KEY);
	input_set_capability(button_input_dev, EV_KEY, KEY_1); //标记设备能够触发的事件

	/*注册输入设备*/
	error = input_register_device(button_input_dev);
	if (0 != error)
	{
		printk(KERN_ERR "input_register_device error");
		gpio_free(button_GPIO_number);
		free_irq(interrupt_number, NULL);
		input_unregister_device(button_input_dev);
		return -1;
	}

	return 0;
}


/*
*驱动注销函数
*/
static void __exit button_driver_exit(void)
{
	printk(KERN_ERR "button_driver_exit\n");

	/*释放申请的引脚,和中断*/
	gpio_free(button_GPIO_number);
	free_irq(interrupt_number, NULL);
	
	/*释放输入子系统相关内容*/
	input_unregister_device(button_input_dev);
}


module_init(button_driver_init);
module_exit(button_driver_exit);

MODULE_LICENSE("GPL");
