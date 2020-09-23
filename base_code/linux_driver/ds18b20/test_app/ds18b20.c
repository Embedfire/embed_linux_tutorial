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

#define    DEV_NAME             ("ds18b20")
#define    DEV_COUNT            (1)

#define    USE_18B20
#define    CMD_SKIP_ROM         (0xcc)
#define    CMD_CONVERT_TEMP     (0x44)
#define    CMD_READ_SCRATCHPAD  (0xbe)


struct ds18b20_dev {
    struct cdev cdev;
    struct class *ds18b20_class;
    struct device_node	*dev_nd;     /* device node */
    dev_t devno;
    int major;
    int minor;
    int temp_pin;

    bool device_open;
    spinlock_t lock;
};

static struct ds18b20_dev *temp_dev = NULL;

static int 
ds18b20_init(void)
{
    int ret = 0;
    unsigned long flags;

    gpio_direction_output(temp_dev->temp_pin,1);
    udelay(100);
    gpio_set_value(temp_dev->temp_pin, 0);
    udelay(460);
    gpio_set_value(temp_dev->temp_pin, 1);
    udelay(100);
    spin_lock_irqsave(&temp_dev->lock, flags);
    gpio_direction_input(temp_dev->temp_pin);
    ret = gpio_get_value(temp_dev->temp_pin);
    spin_unlock_irqrestore(&temp_dev->lock, flags);
    return ret;
}

static void 
ds18b20_write8(unsigned char data)
{
    int i;
    unsigned long flags;

    spin_lock_irqsave(&temp_dev->lock, flags);
    gpio_direction_output(temp_dev->temp_pin,1);
    for (i = 0; i < 8; ++i)
    {   
        gpio_set_value(temp_dev->temp_pin, 0);
        udelay(1);
        if (data & 0x01)
        {
            gpio_set_value(temp_dev->temp_pin, 1);
        }
        data >>= 1;
        udelay(60);
        gpio_set_value(temp_dev->temp_pin, 1);
        udelay(16);
    }
    gpio_set_value(temp_dev->temp_pin, 1);
    spin_unlock_irqrestore(&temp_dev->lock, flags);
}


static unsigned char 
ds18b20_read8(void)
{
    int i;
    unsigned char ret = 0;
    unsigned long flags;

    for (i = 0; i < 8; ++i)
    {   
        spin_lock_irqsave(&temp_dev->lock, flags);
        gpio_direction_output(temp_dev->temp_pin,0);
        udelay(1);
        gpio_set_value(temp_dev->temp_pin, 1);

        gpio_direction_input(temp_dev->temp_pin);
        ret >>= 1;
        if (gpio_get_value(temp_dev->temp_pin))
        {
            ret |= 0x80;
        }
        spin_unlock_irqrestore(&temp_dev->lock, flags);
        udelay(60);
    }

    gpio_direction_output(temp_dev->temp_pin,1);

    return ret;
}

static int 
ds18b20_open(struct inode *inode, struct file *filp)
{
	filp->private_data = temp_dev; 	/* 设置私有数据 */
	if((temp_dev != NULL) && !temp_dev->device_open)
	{
        temp_dev->device_open = true;
		try_module_get(THIS_MODULE);
	}
	else
		return -EFAULT;


	return 0;
}

static ssize_t 
ds18b20_read(struct file *filp, char *buf, size_t len, loff_t *offset)
{
    int ret = 0;
    int lenth = 0;
    unsigned char LSB, MSB;
    char kerBuf[2];
    char userBuf[20];
    struct ds18b20_dev *dev = filp->private_data;

    if(!dev->device_open)	return -EFAULT;

    if(ds18b20_init() != 0){
        ret = -EFAULT;
    }

    udelay(420);
    ds18b20_write8(CMD_SKIP_ROM);   //跳过ROM寻址
    ds18b20_write8(CMD_CONVERT_TEMP);   //This command begins a temperature conversion

    mdelay(750);
    if(ds18b20_init() != 0){
        ret = -EFAULT;
    }

    udelay(400);
    ds18b20_write8(CMD_SKIP_ROM);   //跳过ROM寻址
    ds18b20_write8(CMD_READ_SCRATCHPAD);    //This command copies the scratchpad into the E2 memory of the DS18B20, storing the temperature trigger bytes in nonvolatile memory.

    LSB = ds18b20_read8();
    MSB = ds18b20_read8();

    *(kerBuf + 0) = (LSB >> 4) | (MSB << 4);

    *(kerBuf + 1) = (LSB & 0x0f) * 100 >> 4 ;
    
    // ret = copy_to_user(buf, kerBuf, sizeof(kerBuf));
    lenth = strlen(userBuf);
    sprintf((char *)userBuf, "%d.%dC\r\n", kerBuf[0], kerBuf[1]);
    // printk(KERN_EMERG "%s", userBuf);
    ret = copy_to_user(buf, userBuf, lenth);
    // printk(KERN_EMERG "%d.%d C\r\n", kerBuf[0], kerBuf[1]);
    if (ret) {
		ret = -EFAULT;
    }

    return lenth;
}

static ssize_t 
ds18b20_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	struct ds18b20_dev *dev = filp->private_data;

	if(!dev->device_open)	return -EFAULT;

	if (buf != NULL)
	{
		/* code */
	}
	
	return 0;
}

static long 
ds18b20_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int	nResult = 0;
    struct ds18b20_dev *dev = filp->private_data;
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
ds18b20_release(struct inode *inode, struct file *filp)
{
    struct ds18b20_dev *dev = filp->private_data;
	if(dev->device_open == true) {
		dev->device_open = false;
		module_put(THIS_MODULE);
	}
	return 0;
}

static struct file_operations ds18b20_fops =
{
    .owner   = THIS_MODULE,
    .open    = ds18b20_open,
    .read    = ds18b20_read,
    .write   = ds18b20_write,
    .unlocked_ioctl   = ds18b20_ioctl,
    .release = ds18b20_release,
};

static int 
ds18b20_probe(struct platform_device *pdev)
{
    int result;

    temp_dev = kzalloc(sizeof(struct ds18b20_dev), GFP_KERNEL);
	if (!temp_dev) {
		result = -ENOMEM;
		goto err_free_mem;
	}

    temp_dev->devno = MKDEV(temp_dev->major, 0);
    if (temp_dev->major) {	/*  defined device number */
		result = register_chrdev_region(temp_dev->devno, DEV_COUNT, DEV_NAME);
	} else {    /* not define device number*/
		result = alloc_chrdev_region(&temp_dev->devno, 0, DEV_COUNT, DEV_NAME);	/* 申请设备号 */
		temp_dev->major = MAJOR(temp_dev->devno);	
		temp_dev->minor = MINOR(temp_dev->devno);	
	}

    if (result < 0) {
		// printk(KERN_ERR "alloc_chrdev_region() failed for ds18b20\n");
		return -EINVAL;;
	}

    cdev_init(&temp_dev->cdev, &ds18b20_fops);
    temp_dev->cdev.owner = THIS_MODULE;
    temp_dev->cdev.ops = &ds18b20_fops;
    result = cdev_add(&temp_dev->cdev, temp_dev->devno, 1);
    if (result)
    {
        // printk(KERN_ERR "cdev add failed\n");
        goto fail1;
    }

    temp_dev->ds18b20_class = class_create(THIS_MODULE, "ds18b20_class");
    if (IS_ERR(temp_dev->ds18b20_class))
    {
        // printk(KERN_ERR "class create failed\n");
        goto fail2;
    }

    device_create(temp_dev->ds18b20_class, NULL, temp_dev->devno, DEV_NAME, DEV_NAME);

    temp_dev->dev_nd = of_find_node_by_path("/ds18b20");
	if (temp_dev->dev_nd== NULL) {
        // printk(KERN_EMERG "of_find_node_by_path failed \r\n");
        goto fail2;
	}
	
	// printk(KERN_EMERG "of_find_node_by_path OK \r\n");
#ifdef USE_18B20
	temp_dev->temp_pin = of_get_named_gpio(temp_dev->dev_nd ,"ds18b20_pin", 0);
	if (temp_dev->temp_pin < 0) {
		// printk(KERN_EMERG "can't get ds18b20_pin\r\n");
        goto fail2;
	}

    result = gpio_request(temp_dev->temp_pin , "ds18b20_pin");  
	if(result) {
		// printk(KERN_EMERG "gpio_request for ds18b20_pin is failed!\n");
        goto fail2;
	}
    printk(KERN_EMERG "ds18b20_probe %d\r\n", result);
#endif

    spin_lock_init(&temp_dev->lock);

    return 0;

fail2:
    cdev_del(&temp_dev->cdev);
fail1:
    unregister_chrdev_region(temp_dev->devno, 1);
err_free_mem:
    kfree(temp_dev);

    printk(KERN_EMERG "ds18b20_probe err \r\n");

    return result;
}

static int 
ds18b20_remove(struct platform_device *pdev)
{
    gpio_free(temp_dev->temp_pin);
    device_destroy(temp_dev->ds18b20_class, temp_dev->devno);
    class_destroy(temp_dev->ds18b20_class);
    cdev_del(&temp_dev->cdev);
    unregister_chrdev_region(temp_dev->devno, 1);

    if(temp_dev != NULL){
        kfree(temp_dev);
        temp_dev = NULL;
    }

    return 0;
}

static const struct of_device_id ds18b20_dt_match[] = {
	{ .compatible = "fire,temp_ds18b20" },
	{ }
};
MODULE_DEVICE_TABLE(of, ds18b20_dt_match);

static struct platform_driver ds18b20_driver = {
	.probe		= ds18b20_probe,
	.remove		= ds18b20_remove,
	.driver		= {
		.name	= "fire,temp_ds18b20",
		.of_match_table = of_match_ptr(ds18b20_dt_match),
	},
};


module_platform_driver(ds18b20_driver);

// module_init(ds18b20_init);
// module_exit(0);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("embedfire-jason");
MODULE_DESCRIPTION("imx6ull ds18b20 driver");






/*

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

*/




