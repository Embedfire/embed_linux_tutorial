#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/major.h>
#include <linux/efi.h>
#include <linux/wait.h>
#include <linux/timer.h>

#define     DEV_NAME             ("dht11")
#define     DEV_COUNT            (1)

#define     USE_DHT11
#define     CMD_SKIP_ROM         (0xcc)
#define     CMD_CONVERT_TEMP     (0x44)
#define     CMD_READ_SCRATCHPAD  (0xbe)

#define     HIGH_LEVEL           (0x01)
#define     LOW_LEVEL            (0x00)
#define     CHECK_OK             (true)
#define     CHECK_ERR            (false)
#define     CHECK_NUM            (5)

static DEFINE_SPINLOCK(dht11_lock);

struct dht11_dev {
    struct cdev cdev;
    struct class *dht11_class;
    struct device_node	*dev_nd;     /* device node */
    dev_t dev_num;
    int major;
    int minor;
    int dht11_pin;

    bool checkFlag; 
    unsigned char buffer[6]; 

    bool device_open;
};

static struct dht11_dev *dht11_dev = NULL;
 

static unsigned char 
dht11_read8(void)
{
    int i = 0;      
    int j;       
    bool feedback = false;      
    unsigned char data = 0;        
    for(j = 0; j < 8; j++) {                		
        i = 0;  		/* 等待DHT11的引脚变为高电平 */		
        while(gpio_get_value(dht11_dev->dht11_pin) == LOW_LEVEL) {  	
            feedback = false;		
            udelay(10);  						
            if(i++ > 5)  				
             break;  		
        }  			
        udelay(30);              			
        i = 0;  		
        while(gpio_get_value(dht11_dev->dht11_pin) == HIGH_LEVEL) { 
            feedback = true; 			
            udelay(10);  						
            if(i++ > 6)  				
             break;  		
        }  		
        data <<= 1;  		
        data |= feedback;      
    }        
    
    return data;
}


static void 
get_tempAndHum(void)  {      
    int j = 0;  	    
    gpio_direction_output(dht11_dev->dht11_pin, LOW_LEVEL);    
    mdelay(20);      
    gpio_direction_output(dht11_dev->dht11_pin, HIGH_LEVEL);      
    udelay(40);  	    
    gpio_direction_input(dht11_dev->dht11_pin);      
    if(gpio_get_value(dht11_dev->dht11_pin) == LOW_LEVEL) {           
        while(gpio_get_value(dht11_dev->dht11_pin) == LOW_LEVEL)  {  /* 等待IO口变为高电平 */   
            udelay(3);                       
            if(j++ > 30)             {  				
                break; // get_tempAndHum %d out of time limit            
            }          
        }          
        j = 0;          
        while(gpio_get_value(dht11_dev->dht11_pin) == HIGH_LEVEL)  {  /* 等待IO口变为低电平 */
            udelay(3);                       
            if(j++ > 30) {                
                break;             
            }          
        }          
        for(j = 0; j < 5; j++) { /* 读取温湿度数据 */        	
            dht11_dev->buffer[j] = dht11_read8();   /* 对读取到的数据进行校验 */  
        }
        dht11_dev->buffer[CHECK_NUM] = dht11_dev->buffer[0]+dht11_dev->buffer[1]+dht11_dev->buffer[2]+dht11_dev->buffer[3];     		/* 判断读到的校验值和计算的校验值是否相同 */        
        if(dht11_dev->buffer[4] == dht11_dev->buffer[CHECK_NUM]) /* 相同则把标志值设为0xff */ {             
            dht11_dev->checkFlag = CHECK_OK;           
        } else  {  /* 不相同则把标志值设为0 */           
            dht11_dev->checkFlag = CHECK_ERR; // get_tempAndHum %d out of time limit                              
        }                         
    }  
}


static int 
dht11_open(struct inode *inode, struct file *filp)
{
	filp->private_data = dht11_dev; 	/* 设置私有数据 */

	if((dht11_dev != NULL) && !dht11_dev->device_open)
	{
        dht11_dev->device_open = true;
		try_module_get(THIS_MODULE);
	}
	else
		return -EFAULT;

	return 0;
}

static ssize_t 
dht11_read(struct file *filp, char *buf, size_t len, loff_t *offset)
{
    int ret = 0;
    unsigned int lenth = 0;
    char userBuf[20];
    unsigned long flags = 0;
    struct dht11_dev *dev = filp->private_data;

    if(!(*dev).device_open)	return -EFAULT;

    spin_lock_irqsave(&dht11_lock, flags);
    get_tempAndHum();  
    spin_unlock_irqrestore(&dht11_lock, flags);
    if(dev->checkFlag == true)  	{  		/* 将读取的温湿度数据拷贝到用户空间 */		
        sprintf((char *)userBuf, "temp:%d, hum:%d\r\n", dev->buffer[2], dev->buffer[0]);
        // printk(KERN_ERR "Temperature : %d, Humi : %d\n", dev->buffer[2], dev->buffer[0]);
        ret = copy_to_user(buf, dev->buffer, sizeof(dev->buffer)); 
        lenth = strlen(userBuf);
        // ret = copy_to_user(buf, userBuf, lenth);  		
        if(ret != 0)	{  			
            // printk(KERN_EMERG "copy to user err\n");  			
            return -EAGAIN;  		
        }else  {    
            return lenth;
        }
    }else {	 	
        return -EAGAIN;	
    }

    return ret;
}

static ssize_t 
dht11_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	struct dht11_dev *dev = filp->private_data;

	if(!dev->device_open)	return -EFAULT;

	if (buf != NULL) {
		/* code */
	}
	
	return 0;
}

static long 
dht11_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int	nResult = 0;
    struct dht11_dev *dev = filp->private_data;
    if(!dev->device_open)	return -EFAULT;
    switch(cmd)
    {
        case 0:

            break;
        default:	break;
    }

    return nResult;
}

static int 
dht11_release(struct inode *inode, struct file *filp)
{
    struct dht11_dev *dev = filp->private_data;
	if(dev->device_open == true) {
		dev->device_open = false;
		module_put(THIS_MODULE);
	}
	return 0;
}

static struct file_operations dht11_fops =
{
    .owner              = THIS_MODULE,
    .open               = dht11_open,
    .read               = dht11_read,
    .write              = dht11_write,
    .unlocked_ioctl     = dht11_ioctl,
    .release            = dht11_release,
};

static int 
dht11_probe(struct platform_device *pdev)
{
    int result;

    dht11_dev = kzalloc(sizeof(struct dht11_dev), GFP_KERNEL);
	if (dht11_dev == NULL) {
		result = -ENOMEM;
		goto fail0;
	}

    dht11_dev->dev_num = MKDEV(dht11_dev->major, 0);
    if (dht11_dev->major) {	/*  defined device number */
		result = register_chrdev_region(dht11_dev->dev_num, DEV_COUNT, DEV_NAME);
	} else {    /* not define device number*/
		result = alloc_chrdev_region(&dht11_dev->dev_num, 0, DEV_COUNT, DEV_NAME);	/* 申请设备号 */
		dht11_dev->major = MAJOR(dht11_dev->dev_num);	
		dht11_dev->minor = MINOR(dht11_dev->dev_num);	
	}

    if (result < 0) {
		// printk(KERN_ERR "alloc_chrdev_region() failed for dht11\n");
		return -EINVAL;;
	}

    cdev_init(&dht11_dev->cdev, &dht11_fops);
    dht11_dev->cdev.owner = THIS_MODULE;
    dht11_dev->cdev.ops = &dht11_fops;
    result = cdev_add(&dht11_dev->cdev, dht11_dev->dev_num, 1);
    if (result)
    {
        // printk(KERN_ERR "cdev add failed\n");
        goto fail1;
    }

    dht11_dev->dht11_class = class_create(THIS_MODULE, "dht11_class");
    if (IS_ERR(dht11_dev->dht11_class))
    {
        // printk(KERN_ERR "class create failed\n");
        goto fail2;
    }

    device_create(dht11_dev->dht11_class, NULL, dht11_dev->dev_num, DEV_NAME, DEV_NAME);
	
	// printk(KERN_EMERG "of_find_node_by_path OK \r\n");
#ifdef USE_DHT11
    dht11_dev->dev_nd = of_find_node_by_path("/dht11");
	if (dht11_dev->dev_nd== NULL) {
        // printk(KERN_EMERG "of_find_node_by_path failed \r\n");
		goto fail2;
	}

	dht11_dev->dht11_pin = of_get_named_gpio(dht11_dev->dev_nd ,"dht11_pin", 0);
	if (dht11_dev->dht11_pin < 0) {
		// printk(KERN_EMERG "can't get dht11_pin\r\n");
		goto fail2;
	}

    result = gpio_request(dht11_dev->dht11_pin , "dht11_pin");  
	if(result) {
		// printk(KERN_EMERG "gpio_request for dht11 is failed!\n");
		goto fail2;
	}

#endif
    spin_lock_init(&dht11_lock);
    
    return 0;

fail2:
    cdev_del(&dht11_dev->cdev);
fail1:
    unregister_chrdev_region(dht11_dev->dev_num, 1);
fail0:
    kfree(dht11_dev);

    return result;
}

static int 
dht11_remove(struct platform_device *pdev)
{
    gpio_free(dht11_dev->dht11_pin);
    device_destroy(dht11_dev->dht11_class, dht11_dev->dev_num);
    class_destroy(dht11_dev->dht11_class);
    cdev_del(&dht11_dev->cdev);
    unregister_chrdev_region(dht11_dev->dev_num, 1);
    if (dht11_dev != NULL)
    {
        kfree(dht11_dev);
        dht11_dev = NULL;
    }

    return 0;
}

static const struct of_device_id dht11_dt_match[] = {
	{.compatible = "fire,temphum_dht11"},
	{ }
};
MODULE_DEVICE_TABLE(of, dht11_dt_match);

static struct platform_driver dht11_driver = {
	.probe		= dht11_probe,
	.remove		= dht11_remove,
	.driver		= {
		.name	= "fire,temphum_dht11",
		.of_match_table = of_match_ptr(dht11_dt_match),
	},
};

module_platform_driver(dht11_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("embedfire-jason");
MODULE_DESCRIPTION("imx6ull dht11 driver");






/*

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

*/




