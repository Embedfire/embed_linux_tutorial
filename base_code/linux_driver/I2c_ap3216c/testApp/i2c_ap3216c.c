#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/kernel.h>
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
#include <linux/platform_device.h>
#include "i2c_ap3216c.h"

/*------------------字符设备内容----------------------*/
#define DEV_NAME				("ap3216c")
#define DEV_CNT					(1)


struct ap3216cdev {
	dev_t 				dev_num;				
	struct cdev 		cdev_ap3216c;			
	struct class 		*class_ap3216c;			
	struct device 		*device_ap3216c;		
	struct device_node	*device_node_ap3216c; 	
	struct i2c_client 	*ap3216c_client;
	void 				*private_data;
	int 				major;					
	int 				minor;					
	unsigned int 		ALS, IR, PS;			

	bool				device_open;
};


/*------------------IIC设备内容----------------------*/

static struct ap3216cdev ap3216cdev_dev;
static struct ap3216cdev *ap3216cdev_p = &ap3216cdev_dev;

/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
static int 
i2c_read_ap3216c( struct i2c_client *client, u8 addr, void *buf, int lenth )
{
	int ret;
	struct i2c_msg send_msg[2];

	/* send_msg[0]:发送要读取的首地址 */
	send_msg[0].addr = client->addr;	
	send_msg[0].flags = 0;				
	send_msg[0].buf = &addr;			
	send_msg[0].len = 1;				

	/* send_msg[1]:读取数据 */
	send_msg[1].addr = client->addr;	
	send_msg[1].flags = I2C_M_RD;		
	send_msg[1].buf = buf;				
	send_msg[1].len = lenth;			

	ret = i2c_transfer(client->adapter, send_msg, 2);
	if(ret != 2) {
		// printk(KERN_DEBUG "i2c_read_ap3216c err\n");
		ret = -1;
	}

	return ret;
}



/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
static int 
i2c_write_ap3216c( struct i2c_client *client, u8 reg, u8 *buf, u8 lenth )
{
	int ret = 0;
	u8 user_data[128];
	struct i2c_msg send_msg;

	*(user_data + 0) = reg;					
	memcpy((user_data + 1), buf,lenth);		

	send_msg.addr = client->addr;		
	send_msg.flags = 0;					
	send_msg.buf = user_data;				
	send_msg.len = lenth + 1;				

	ret = i2c_transfer(client->adapter, &send_msg, 1);
	if (ret != 2) { 
		// printk(KERN_DEBUG "\n i2c_write_ap3216c error \n");
		return -1;
	}
	
	return 0;
}


/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
void 
get_ap3216c_data( void )
{
	unsigned char i = 0;
    unsigned char dataBuf[6];

	while(i < 6) {
		i2c_read_ap3216c(ap3216cdev_p->ap3216c_client, AP3216C_IRDATALOW + i ,(dataBuf + i), 1);
		i += 1;
	}

	(dataBuf[0] & 0X80) ? (ap3216cdev_p->IR = 0) : (ap3216cdev_p->IR = ((unsigned int)dataBuf[1] << 2) | (dataBuf[0] & 0X03));		

	ap3216cdev_p->ALS = ((unsigned int)dataBuf[3] << 8) | dataBuf[2];	/* 读取ALS传感器的数据 	*/  
	/* IR_OF位为1,则数据无效	*/    						/* 读取PS传感器的数据    */
	(dataBuf[4] & 0x40) ? (ap3216cdev_p->PS = 0) : (ap3216cdev_p->PS = ((unsigned int)(dataBuf[5] & 0X3F) << 4) | (dataBuf[4] & 0X0F));
}

/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
static int 
ap3216c_open( struct inode *inode, struct file *filp )
{
	unsigned char cmd[2];
	filp->private_data = ap3216cdev_p;
	cmd[0] = 0x04;
	cmd[1] = 0x03;

	if(ap3216cdev_p != NULL && (!ap3216cdev_p->device_open)) {
		ap3216cdev_p->device_open = true;
		
		try_module_get(THIS_MODULE);
	}
	else
		return -EFAULT;

	i2c_write_ap3216c(ap3216cdev_p->ap3216c_client, AP3216C_SYSTEMCONG, (cmd + 0), 1);
	mdelay(16);														/* AP3216C复位10ms 	*/
	i2c_write_ap3216c(ap3216cdev_p->ap3216c_client, AP3216C_SYSTEMCONG, (cmd + 1), 1);			/* 开启ALS、PS+IR 		*/
	return 0;
}

/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
static ssize_t 
ap3216c_read( struct file *filp, char __user *buf, size_t cnt, loff_t *off )
{
	int ret = 0;
	unsigned int lenth = 0;
	unsigned int data[3];
	char userbuf[128];
	struct ap3216cdev *dev = (struct ap3216cdev *)filp->private_data;

	if(!ap3216cdev_p->device_open)	return -EFAULT;

	get_ap3216c_data();

	data[0] = dev->ALS;
	data[1] = dev->IR;
	data[2] = dev->PS;

	sprintf((char *)userbuf, "Ambient : [%4d], Infrared ray : [%4d], Proximity : [%4d]\r\n",\
						 data[0], data[1], data[2]);
	lenth = strlen(userbuf);
	msleep(200);
	ret = copy_to_user(buf, userbuf, lenth);
	// ret = copy_to_user(buf, data, sizeof(data));
	if(ret != 0) {
		// printk(KERN_DEBUG "copy_to_user error!");
		return -1;
	}

	return lenth;
}


/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
static int 
ap3216c_release( struct inode *inode, struct file *filp )
{
	unsigned char cmd[0];
	cmd[0] = 0x00;

	if((ap3216cdev_p != NULL) && (ap3216cdev_p->device_open != false)) {
		ap3216cdev_p->device_open = false;
		i2c_write_ap3216c(ap3216cdev_p->ap3216c_client, AP3216C_SYSTEMCONG, (cmd + 0), 1);
		module_put(THIS_MODULE);
	}

	return 0;
}


/* AP3216C file_operations函数 */
static const struct file_operations ap3216c_ops = {
	.owner = THIS_MODULE,
	.open = ap3216c_open,
	.read = ap3216c_read,
	.release = ap3216c_release,
};


/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
static int 
ap3216c_probe( struct i2c_client *client, const struct i2c_device_id *id )
{
	int err = 0;
	// ap3216cdev_p = (struct ap3216cdev *)kzalloc(sizeof(struct ap3216cdev), GFP_KERNEL);
	// if (!ap3216cdev_p) {
	// 	err = -ENOMEM;
	// 	goto alloc_err;
	// }

	/* 1、构建设备号 */
	if (ap3216cdev_p->major) {
		ap3216cdev_p->dev_num = MKDEV(ap3216cdev_p->major, 0);
		register_chrdev_region(ap3216cdev_p->dev_num, DEV_CNT, DEV_NAME);
	} else {
		alloc_chrdev_region(&ap3216cdev_p->dev_num, 0, DEV_CNT, DEV_NAME);
		ap3216cdev_p->major = MAJOR(ap3216cdev_p->dev_num);
		ap3216cdev_p->minor = MINOR(ap3216cdev_p->dev_num);
	}

	/* 2、注册设备 */
	cdev_init(&ap3216cdev_p->cdev_ap3216c, &ap3216c_ops);
	cdev_add(&ap3216cdev_p->cdev_ap3216c, ap3216cdev_p->dev_num, DEV_CNT);
	/* 3、创建类 */
	ap3216cdev_p->class_ap3216c = class_create(THIS_MODULE, DEV_NAME);
	/* 4、创建设备 */
	ap3216cdev_p->device_ap3216c = device_create(ap3216cdev_p->class_ap3216c, NULL, ap3216cdev_p->dev_num, NULL, DEV_NAME);
	ap3216cdev_p->private_data = client;
	ap3216cdev_p->ap3216c_client = client;

	ap3216cdev_p->device_open = false;
	return err;
}



/*************************************************
Function:         
Description:             // 函数功能、性能等的描述
Calls:                   // 被本函数调用的函数清单
Input:                     // 输入参数说明，包括每个参数的作
Output:                  // 对输出参数的说明。
Return:                   // 函数返回值的说明
Others:                  // 其它说明
*************************************************/
static int 
ap3216c_remove( struct i2c_client *client )

{
	/* 注销掉类和设备 */
	device_destroy(ap3216cdev_p->class_ap3216c, ap3216cdev_p->dev_num);
	class_destroy(ap3216cdev_p->class_ap3216c);
	cdev_del(&ap3216cdev_p->cdev_ap3216c);
	unregister_chrdev_region(ap3216cdev_p->dev_num, DEV_CNT);
	return 0;
}


/* 传统匹配方式ID列表 */
static const struct i2c_device_id ap3216c_dev_id[] = {
	{"fire,i2c_ap3216c", 0},  
	{/* Sentinel */}
};

/* 设备树匹配列表 */
static const struct of_device_id ap3216c_of_match[] = {
	{.compatible = "fire,i2c_ap3216c"},
	{/* Sentinel */}
};

/* i2c驱动结构体 */	
static struct i2c_driver ap3216c_driver = {
	.probe = ap3216c_probe,
	.remove = ap3216c_remove,
	.id_table = ap3216c_dev_id,
	.driver = {
		.name = "fire,i2c_ap3216c",
		.owner = THIS_MODULE,
		.of_match_table = ap3216c_of_match, 
	},
};
module_i2c_driver(ap3216c_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("I.MX6ULL ap3216c driver");
MODULE_AUTHOR("embedfire");
MODULE_ALIAS("platform:ap3216c");

/*

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

*/