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

#define     DEV_NAME             ("infra")
#define     DEV_COUNT            (1)

#define     USE_INFRA
#define     CMD_SKIP_ROM         (0xcc)
#define     CMD_CONVERT_TEMP     (0x44)
#define     CMD_READ_SCRATCHPAD  (0xbe)

#define     HIGH_LEVEL           (0x01)
#define     LOW_LEVEL            (0x00)
#define     CHECK_OK             (true)
#define     CHECK_ERR            (false)
#define     CHECK_NUM            (5)

#define  IrDa_DATA_IN()	   gpio_get_value(infra_dev_p->infra_pin) 
#define IRDA_ID 0  

/*********************type define***********************/
static enum {RESET = 0, SET = !RESET};
    /* exact-width signed integer types */
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;

   /* exact-width unsigned integer types */
typedef   unsigned          char uint8_t;
typedef   unsigned short     int uint16_t;
typedef   unsigned           int uint32_t;


struct infradev {
    struct cdev cdev;
    struct class *infra_class;
    struct device_node	*dev_nd;     /* device node */
    struct work_struct infra_down_wq;
    dev_t dev_num;
    int major;
    int minor;
    int infra_pin;
    int IRQnum;

    int flag;                   //表示数据帧的开始
    int num;                    //表示数据帧里的第几位数据
    long long prev;      //记录上次的时间
    unsigned int times[40];     //记录每位数据的时间

    bool checkFlag; 
    unsigned char buffer[6]; 

    uint32_t    frame_data;  
    uint8_t     frame_cnt;
    uint8_t     frame_flag;  
    uint8_t     isr_cnt; 
    uint8_t     key_val;

    bool device_open;
};

static struct infradev *infra_dev_p = NULL;
// static DEFINE_SPINLOCK(infra_lock);

static int 
infra_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	filp->private_data = infra_dev_p; 	/* 设置私有数据 */

	if((infra_dev_p != NULL) && !infra_dev_p->device_open)
	{
        infra_dev_p->device_open = true;
		try_module_get(THIS_MODULE);
	}
	else
		return -EFAULT;

	return ret;
}

static ssize_t 
infra_read(struct file *filp, char *buf, size_t len, loff_t *offset)
{
    int ret = 0;
    // unsigned long flags = 0;
    struct infradev *dev = filp->private_data;

    if(!(*dev).device_open)	return -EFAULT;

    // spin_lock_irqsave(&infra_lock, flags);

    // spin_unlock_irqrestore(&infra_lock, flags);
	
    // ret = copy_to_user(buf, dev->buffer, sizeof(dev->buffer));  		


    return ret;
}

static ssize_t 
infra_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
	struct infradev *dev = filp->private_data;

	if(!dev->device_open)	return -EFAULT;

	if (buf != NULL) {
		/* code */
	}
	
	return 0;
}

static long 
infra_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int	nResult = 0;
    struct infradev *dev = filp->private_data;
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
infra_release(struct inode *inode, struct file *filp)
{
    struct infradev *dev = filp->private_data;
	if(dev->device_open == true) {
		dev->device_open = false;
		module_put(THIS_MODULE);
	}
	return 0;
}

static struct file_operations infra_fops =
{
    .owner              = THIS_MODULE,
    .open               = infra_open,
    .read               = infra_read,
    .write              = infra_write,
    .unlocked_ioctl     = infra_ioctl,
    .release            = infra_release,
};

/* get hight level time */
uint8_t Get_Pulse_Time(void)
{
  uint8_t time = 0;
  while( IrDa_DATA_IN() )
  {
    time ++;
    udelay(20);     // delay 20us
    if(time == 250)
      return time;   // out of time overflow   
  }
  return time;
}


/*
 * IrDa process function
 */
uint8_t IrDa_Process(void)
{
  uint8_t first_byte, sec_byte, tir_byte, fou_byte;  
  
  first_byte =  infra_dev_p->frame_data >> 24;
  sec_byte = ( infra_dev_p->frame_data>>16) & 0xff;
  tir_byte =  infra_dev_p->frame_data >> 8;
  fou_byte =  infra_dev_p->frame_data;
  
  /* ¼ÇµÃÇå±êÖ¾Î» */
   infra_dev_p->frame_flag = 0;
  
  if( (first_byte==(uint8_t)~sec_byte) && (first_byte==IRDA_ID) )
  {
    if( tir_byte == (u8)~fou_byte )
      return tir_byte;
  }
  
  return 0;   /* ´íÎó·µ»Ø */
}

static irqreturn_t 
Infra_handler(int irq, void *dev_id)
{
	// struct infradev *dev = (struct infradev *)dev_id;
    uint8_t pulse_time = 0;
    uint8_t leader_code_flag = 0; /* Òýµ¼Âë±êÖ¾Î»£¬µ±Òýµ¼Âë³öÏÖÊ±£¬±íÊ¾Ò»Ö¡Êý¾Ý¿ªÊ¼ */
    uint8_t irda_data = 0;        /* Êý¾ÝÔÝ´æÎ» */

    while(true) {

        if( IrDa_DATA_IN() == SET ) /* Ö»²âÁ¿¸ßµçÆ½µÄÊ±¼ä */ {       
            // printk(KERN_ERR "Inter Infra_handler!\n");
            pulse_time = Get_Pulse_Time();

            /* >=5ms ²»ÊÇÓÐÓÃÐÅºÅ µ±³öÏÖ¸ÉÈÅ»òÕßÁ¬·¢ÂëÊ±£¬Ò²»ábreakÌø³öwhile(1)Ñ­»· */
            if( pulse_time >= 250 ) {
                goto exit; /* Ìø³öwhile(1)Ñ­»· */
            }

            if(pulse_time>=200 && pulse_time<250)         /* »ñµÃÇ°µ¼Î» 4ms~4.5ms */ {
                leader_code_flag = 1;
            }
            else if(pulse_time>=10 && pulse_time<50)      /* 0.56ms: 0.2ms~1ms */ {
                irda_data = 0;
            }
            else if(pulse_time>=50 && pulse_time<100)     /* 1.68ms£º1ms~2ms */ {
                irda_data =1 ; 
            }        
            else if( pulse_time>=100 && pulse_time<=200 ) /* 2.1ms£º2ms~4ms */ {/* Á¬·¢Âë£¬ÔÚµÚ¶þ´ÎÖÐ¶Ï³öÏÖ */
                 infra_dev_p->frame_flag = 1;               /* Ò»Ö¡Êý¾Ý½ÓÊÕÍê³É */
                 infra_dev_p->frame_cnt++;                  /* °´¼ü´ÎÊý¼Ó1 */
                 infra_dev_p->isr_cnt ++;                   /* ½øÖÐ¶ÏÒ»´Î¼Ó1 */
                goto exit;                        /* Ìø³öwhile(1)Ñ­»· */
            }

            if( leader_code_flag == 1 ) {/* ÔÚµÚÒ»´ÎÖÐ¶ÏÖÐÍê³É */
                 infra_dev_p->frame_data <<= 1;
                 infra_dev_p->frame_data += irda_data;
                 infra_dev_p->frame_cnt = 0;
                 infra_dev_p->isr_cnt = 1;
            }
        } 
    }

    // printk(KERN_ERR "Inter Infra_handler!\n");
exit:
    schedule_work(&infra_dev_p->infra_down_wq);
	return IRQ_RETVAL(IRQ_HANDLED);
}


void
infra_do_work(struct work_struct * work)
{
    if(  infra_dev_p->frame_flag == 1 ) /* Ò»Ö¡ºìÍâÊý¾Ý½ÓÊÕÍê³É */
    {
         infra_dev_p->key_val = IrDa_Process();
        printk(KERN_ERR "\r\n remote_val : [%d] \r\n", infra_dev_p->key_val);
        printk(KERN_ERR "\r\n push num : [%d] \r\n", infra_dev_p->frame_cnt);
        printk(KERN_ERR "\r\n interrupt num : [%d] \r\n", infra_dev_p->isr_cnt);
        
        /* ²»Í¬µÄÒ£¿ØÆ÷Ãæ°å¶ÔÓ¦²»Í¬µÄ¼üÖµ£¬ÐèÒªÊµ¼Ê²âÁ¿ */
        switch(  infra_dev_p->key_val )
        {
            case 0:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n Error \r\n");
            break;
            
            case 162:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n POWER \r\n");
            break;
            
            case 226:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n MENU \r\n");
            break;
            
            case 34:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n TEST \r\n");
            break;
            
            case 2:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n + \r\n");
            break;
            
            case 194:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n RETURN \r\n");
            break;
            
            case 224:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n |<< \r\n");
            break;
            
            case 168:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n > \r\n");
            break;
            
            case 144:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n >>| \r\n");
            break;
            
            case 104:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 0 \r\n");
            break;
            
            case 152:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n - \r\n");
            break;
            
            case 176:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n C \r\n");
            break;
            
            case 48:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 1 \r\n");
            break;
            
            case 24:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 2 \r\n");
            break;
            
            case 122:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 3 \r\n");
            break;
            
            case 16:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 4 \r\n");
            break;
            
            case 56:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 5 \r\n");
            break;
            
            case 90:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 6 \r\n");
            break;
            
            case 66:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 7 \r\n");
            break;
            
            case 74:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 8 \r\n");
            break;
            
            case 82:
            printk(KERN_ERR "\r\n remote_val=%d \r\n", infra_dev_p->key_val);
            printk(KERN_ERR "\r\n 9 \r\n");
            break;
            
            default:       
            break;
        }      

    }
}


static int 
infra_probe(struct platform_device *pdev)
{
    int result;

    infra_dev_p = kzalloc(sizeof(struct infradev), GFP_KERNEL);
	if (infra_dev_p == NULL) {
		result = -ENOMEM;
		goto alloc_err;
	}

    infra_dev_p->dev_num = MKDEV(infra_dev_p->major, 0);
    if (infra_dev_p->major) {	/*  defined device number */
		result = register_chrdev_region(infra_dev_p->dev_num, DEV_COUNT, DEV_NAME);
	} else {    /* not define device number*/
		result = alloc_chrdev_region(&infra_dev_p->dev_num, 0, DEV_COUNT, DEV_NAME);	/* 申请设备号 */
		infra_dev_p->major = MAJOR(infra_dev_p->dev_num);	
		infra_dev_p->minor = MINOR(infra_dev_p->dev_num);	
	}

    if (result < 0) {
		// printk(KERN_ERR "alloc_chrdev_region() failed for infra\n");
		return -EINVAL;;
	}

    cdev_init(&infra_dev_p->cdev, &infra_fops);
    infra_dev_p->cdev.owner = THIS_MODULE;
    infra_dev_p->cdev.ops = &infra_fops;
    result = cdev_add(&infra_dev_p->cdev, infra_dev_p->dev_num, DEV_COUNT);
    if (result)
    {
        // printk(KERN_ERR "cdev add failed\n");
        goto fail1;
    }

    infra_dev_p->infra_class = class_create(THIS_MODULE, "infra_class");
    if (IS_ERR(infra_dev_p->infra_class))
    {
        // printk(KERN_ERR "class create failed\n");
        goto fail2;
    }

    device_create(infra_dev_p->infra_class, NULL, infra_dev_p->dev_num, DEV_NAME, DEV_NAME);
	
	// printk(KERN_EMERG "of_find_node_by_path OK \r\n");
#ifdef USE_INFRA
    infra_dev_p->dev_nd = of_find_node_by_path("/infra");
	if (infra_dev_p->dev_nd== NULL) {
        // printk(KERN_EMERG "of_find_node_by_path failed \r\n");
		goto fail2;
	}

	infra_dev_p->infra_pin = of_get_named_gpio(infra_dev_p->dev_nd ,"infra_pin", 0);
	if (infra_dev_p->infra_pin < 0) {
		// printk(KERN_EMERG "can't get infra_pin\r\n");
		goto fail2;
	}

    result = gpio_request(infra_dev_p->infra_pin , "infra_pin");  
	if(result) {
		// printk(KERN_EMERG "gpio_request for infra is failed!\n");
		goto fail2;
	}

    gpio_direction_input(infra_dev_p->infra_pin);
    infra_dev_p->IRQnum = gpio_to_irq(infra_dev_p->infra_pin);
    // printk(KERN_EMERG "pin num : %d, irqnum=%d\r\n", infra_dev_p->infra_pin, infra_dev_p->IRQnum);

    result = request_irq(infra_dev_p->IRQnum, Infra_handler, 
                        IRQF_TRIGGER_FALLING, "infra_pin", infra_dev_p);
    if(result < 0){
        // printk(KERN_EMERG "irq %d request failed!\r\n", infra_dev_p->IRQnum);
        return -EFAULT;
    }

    INIT_WORK(&infra_dev_p->infra_down_wq, infra_do_work);

    infra_dev_p->flag = 0;
    infra_dev_p->num = 0;
    infra_dev_p->prev = -1;
    memset(infra_dev_p->times, 0, sizeof(infra_dev_p->times));

#endif

    // spin_lock_init(&infra_lock);
    
    return 0;

fail2:
    cdev_del(&infra_dev_p->cdev);
fail1:
    unregister_chrdev_region(infra_dev_p->dev_num, 1);
alloc_err:
    kfree(infra_dev_p);

    return result;
}

static int 
infra_remove(struct platform_device *pdev)
{

    free_irq(infra_dev_p->IRQnum, infra_dev_p);
    gpio_free(infra_dev_p->infra_pin);
    device_destroy(infra_dev_p->infra_class, infra_dev_p->dev_num);
    class_destroy(infra_dev_p->infra_class);
    cdev_del(&infra_dev_p->cdev);
    unregister_chrdev_region(infra_dev_p->dev_num, 1);
    if (infra_dev_p != NULL) {
        kfree(infra_dev_p);
        infra_dev_p = NULL;
    }

    return 0;
}

static const struct of_device_id infra_dt_match[] = {
	{ .compatible = "fire,InfraredRay" },
	{ }
};
MODULE_DEVICE_TABLE(of, infra_dt_match);

static struct platform_driver infra_driver = {
	.probe		= infra_probe,
	.remove		= infra_remove,
	.driver		= {
		.name	= "fire,InfraredRay",
		.of_match_table = of_match_ptr(infra_dt_match),
	},
};

module_platform_driver(infra_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("embedfire-jason");
MODULE_DESCRIPTION("imx6ull infra driver");






/*

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

*/


/* @description		: 中断服务函数，开启定时器，延时10ms，
 *				  	  定时器用于按键消抖。
 * @param - irq 	: 中断号 
 * @param - dev_id	: 设备结构。
 * @return 			: 中断执行结果
 */
// static irqreturn_t 
// Infra_handler(int irq, void *dev_id)
// {
//     unsigned int offset;
//     int i, j, tmp;
// 	struct infradev *dev = (struct infradev *)dev_id;
//     long long now = ktime_to_us(ktime_get());

//     if (!dev->flag) {//数据开始
//         dev->flag = 1;
//         dev->prev = now;
        
//         goto exit;
//     }
//     offset = now - dev->prev;
//     dev->prev = now;
    
//     // printk(KERN_CRIT "----------OFFSET:%d\n", offset);
//     if ((offset > 13000) && (offset < 14000)) {//判断是否收到引导码，引导码13.5ms
//         dev->num = 0;
//         printk(KERN_CRIT "++++++++++++++++++++引导码，引导码13.5ms+++++++++++++!\n");
//         goto exit;
//     }   

//     //不是引导码时间，数据位时间
//     if (dev->num < 32)
//         dev->times[dev->num++] = offset;

//     if (dev->num >= 32) {
//         for (i = 0; i < 4; i++) //共4个字节
//         {
//             tmp = 0;
//             for (j = 0; j < 8; j++) //每字节8位
//             {
//                 if (dev->times[i*8+j] > 2000) //如果数据位的信号周期大于2ms, 则是二进制数据1
//                     tmp |= 1<<j;
//             }
//             printk(KERN_CRIT "%02x", tmp);
//         }
//         printk(KERN_CRIT "---------------------------\n");
//         dev->flag = 0; //重新开始帧
//     }

//     // printk(KERN_ERR "Inter Infra_handler!\n");
// exit:
// 	return IRQ_RETVAL(IRQ_HANDLED);
// }

// static irqreturn_t 
// Infra_handler(int irq, void *dev_id)
// {
//     unsigned int offset;
//     int i, j, tmp;
// 	struct infradev *dev = (struct infradev *)dev_id;
//     long long now = ktime_to_us(ktime_get());
//     offset = now - dev->prev;

//     if (-1 == dev->prev) {//数据开始
//         dev->prev = now;
//         goto exit;
//     }
    
//     dev->prev = now;
    
//     // 
//     if ((offset > 13000) && (offset < 14000)) {//判断是否收到引导码，引导码13.5ms
//         dev->num = 0;
//         dev->prev = now;
//         // printk(KERN_CRIT "----------OFFSET:%d\n", offset);
       
//         goto exit;
//     }   

//     //不是引导码时间，数据位时间
//     dev->times[dev->num++] = offset;

//     if (dev->num > 32) {
//         for (i = 0; i < 4; i++) //共4个字节
//         {
//             tmp = 0;
//             for (j = 0; j < 8; j++) //每字节8位
//             {   
//                 printk(KERN_CRIT "  %d\r\n", dev->times[i*8+j]);
//                 if (dev->times[i*8+j] > 2000) //如果数据位的信号周期大于2ms, 则是二进制数据1
//                     tmp |= 1<<j;
//             }
//             printk(KERN_CRIT "%02x", tmp);
//         }
//         printk(KERN_CRIT "-----\n");
//         dev->prev = -1;
//         dev->num = 0;
//     }

//     // printk(KERN_ERR "Inter Infra_handler!\n");
// exit:
// 	return IRQ_RETVAL(IRQ_HANDLED);
// }

