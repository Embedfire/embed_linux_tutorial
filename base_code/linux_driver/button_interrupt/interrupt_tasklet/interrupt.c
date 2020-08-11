
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

#include "interrupt.h"




/*------------------字符设备内容----------------------*/
#define DEV_NAME "button"
#define DEV_CNT (1)

static dev_t button_devno;		 //定义字符设备的设备号
static struct cdev button_chr_dev; //定义字符设备结构体chr_dev
struct class *class_button;		 //保存创建的类
struct device *device_button;		 // 保存创建的设备


struct device_node	*button_device_node = NULL;  //定义按键设备节点结构体
unsigned  button_GPIO_number = 0;  //保存button使用的GPIO引脚编号
u32  interrupt_number = 0;         // button 引脚中断编号
atomic_t   button_status = ATOMIC_INIT(0);  //定义整型原子变量，保存按键状态 ，设置初始值为0


struct tasklet_struct button_tasklet;


void button_tasklet_hander(unsigned long data)
{
	int counter = 1;
	mdelay(200);
	printk(KERN_ERR "button_tasklet_hander counter = %d  \n", counter++);
	mdelay(200);
	printk(KERN_ERR "button_tasklet_hander counter = %d  \n", counter++);
	mdelay(200);
	printk(KERN_ERR "button_tasklet_hander counter = %d  \n", counter++);
	mdelay(200);
	printk(KERN_ERR "button_tasklet_hander counter = %d \n", counter++);
	mdelay(200);
	printk(KERN_ERR "button_tasklet_hander counter = %d \n", counter++);
}



static irqreturn_t button_irq_hander(int irq, void *dev_id)
{
	printk(KERN_ERR "button_irq_hander----------inter");
	/*按键状态加一*/
	atomic_inc(&button_status);

	tasklet_schedule(&button_tasklet);

	printk(KERN_ERR "button_irq_hander-----------exit");
	return IRQ_HANDLED;
}




static int button_open(struct inode *inode, struct file *filp)
{
	int error = -1;

	/*获取按键 设备树节点*/
	button_device_node = of_find_node_by_path("/button_interrupt");
	if(NULL == button_device_node)
	{
		printk("of_find_node_by_path error!");
		return -1;
	}

	/*获取按键使用的GPIO*/
	button_GPIO_number = of_get_named_gpio(button_device_node ,"button_gpio", 0);
	if(0 == button_GPIO_number)
	{
		printk("of_get_named_gpio error");
		return -1;
	}

	/*申请GPIO  , 记得释放*/
	error = gpio_request(button_GPIO_number, "button_gpio");
	if(error < 0)
	{
		printk("gpio_request error");
		gpio_free(button_GPIO_number);
		return -1;
	}

	error = gpio_direction_input(button_GPIO_number);//设置引脚为输入模式

	/*获取中断号*/
	interrupt_number = irq_of_parse_and_map(button_device_node, 0);
	printk("\n irq_of_parse_and_map! =  %d \n",interrupt_number);

	/*申请中断, 记得释放*/
	error = request_irq(interrupt_number,button_irq_hander,IRQF_TRIGGER_RISING,"button_interrupt",device_button);
	if(error != 0)
	{
		printk("request_irq error");
		free_irq(interrupt_number, device_button);
		return -1;
	}


	/*初始化button_tasklet*/
	tasklet_init(&button_tasklet,button_tasklet_hander,0);

	/*申请之后已经开启了，切记不要再次打开，否则运行时报错*/
	// // enable_irq(interrupt_number);

	return 0;

}

static int button_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	int error = -1;
	int button_countervc = 0;

	/*读取按键状态值*/
	button_countervc = atomic_read(&button_status);

	/*结果拷贝到用户空间*/
	error = copy_to_user(buf, &button_countervc, sizeof(button_countervc));
	if(error < 0)
	{
		printk("copy_to_user error");
		return -1;
	}

	/*清零按键状态值*/
	atomic_set(&button_status,0);
	return 0;
}

/*字符设备操作函数集，.release函数实现*/
static int button_release(struct inode *inode, struct file *filp)
{
	/*释放申请的引脚,和中断*/
	gpio_free(button_GPIO_number);
	free_irq(interrupt_number, device_button);
	return 0;
}



/*字符设备操作函数集*/
static struct file_operations button_chr_dev_fops = {
	.owner = THIS_MODULE,
	.open = button_open,
	.read = button_read,
	.release = button_release};

/*
*驱动初始化函数
*/
static int __init button_driver_init(void)
{
	int error = -1;

	/*采用动态分配的方式，获取设备编号，次设备号为0，*/
	error = alloc_chrdev_region(&button_devno, 0, DEV_CNT, DEV_NAME);
	if (error < 0)
	{
		printk("fail to alloc button_devno\n");
		goto alloc_err;
	}

	/*关联字符设备结构体cdev与文件操作结构体file_operations*/
	button_chr_dev.owner = THIS_MODULE;
	cdev_init(&button_chr_dev, &button_chr_dev_fops);

	/*添加设备至cdev_map散列表中*/ 
	error = cdev_add(&button_chr_dev, button_devno, DEV_CNT);
	if (error < 0) 
	{
		printk("fail to add cdev\n");
		goto add_err;
	}

	class_button = class_create(THIS_MODULE, DEV_NAME);                         //创建类
	device_button = device_create(class_button, NULL, button_devno, NULL, DEV_NAME);//创建设备 DEV_NAME 指定设备名，

	return 0;

add_err:
	unregister_chrdev_region(button_devno, DEV_CNT);    // 添加设备失败时，需要注销设备号
	printk("\n error! \n");
	
alloc_err:
	return -1;
}



/*
*驱动注销函数
*/
static void __exit button_driver_exit(void)
{
	pr_info("button_driver_exit\n");
	/*删除设备*/
	device_destroy(class_button, button_devno);		   //清除设备
	class_destroy(class_button);					   //清除类
	cdev_del(&button_chr_dev);					       //清除设备号
	unregister_chrdev_region(button_devno, DEV_CNT);   //取消注册字符设备
}



module_init(button_driver_init);
module_exit(button_driver_exit);

MODULE_LICENSE("GPL");




