#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>


#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/io.h>

#include <linux/of.h>
#include <linux/of_address.h>


#define DEV_NAME            "of_test"
#define DEV_CNT                 (1)
//定义字符设备的设备号
static dev_t led_devno;
//定义字符设备结构体chr_dev
static struct cdev led_chr_dev;
//创建类
struct class *led_chrdev_class;


struct device_node	*led_device_node; //led的设备树节点
struct device_node  *rgb_led_red_device_node; //rgb_led_red 红灯节点
struct property     *rgb_led_red_property;    //定义属性结构体指针
int size = 0 ;
unsigned int out_values[18];  //保存读取得到的REG 属性值

/*.open 函数*/
static int led_chr_dev_open(struct inode *inode, struct file *filp)
{
    int error_status = -1;

    printk("\n open form device \n");

    /*of 测试函数*/
    led_device_node = of_find_node_by_path("/led");
    if(led_device_node == NULL)
    {
        printk(KERN_EMERG "\n get led_device_node failed ! \n");
        return -1;
    }
    /*根据 led_device_node 设备节点结构体输出节点的基本信息*/
    printk(KERN_EMERG "name: %s",led_device_node->name); //输出节点名
    printk(KERN_EMERG "child name: %s",led_device_node->child->name);  //输出子节点的节点名


    /*获取 rgb_led_red_device_node 的子节点*/ 
    rgb_led_red_device_node = of_get_next_child(led_device_node,NULL); 
    if(rgb_led_red_device_node == NULL)
    {
        printk(KERN_EMERG "\n get rgb_led_red_device_node failed ! \n");
        return -1;
    }
    printk(KERN_EMERG "name: %s",rgb_led_red_device_node->name); //输出节点名
    printk(KERN_EMERG "parent name: %s",rgb_led_red_device_node->parent->name);  //输出父节点的节点名


    /*获取 rgb_led_red_device_node 节点  的"compatible" 属性 */ 
    rgb_led_red_property = of_find_property(rgb_led_red_device_node,"compatible",&size);
    if(rgb_led_red_property == NULL)
    {
        printk(KERN_EMERG "\n get rgb_led_red_property failed ! \n");
        return -1;
    }
    printk(KERN_EMERG "size = : %d",size);                      //实际读取得到的长度
    printk(KERN_EMERG "name: %s",rgb_led_red_property->name);   //输出属性名
    printk(KERN_EMERG "length: %d",rgb_led_red_property->length);        //输出属性长度
    printk(KERN_EMERG "value : %s",(char*)rgb_led_red_property->value);  //属性值


    /*获取 reg 地址属性*/
    error_status = of_property_read_u32_array(rgb_led_red_device_node,"reg",out_values, 2);
    if(error_status != 0)
    {
        printk(KERN_EMERG "\n get out_values failed ! \n");
        return -1;
    }
    printk(KERN_EMERG"0x%08X ", out_values[0]);
    printk(KERN_EMERG"0x%08X ", out_values[1]);

    return 0;
}

/*.release 函数*/
static int led_chr_dev_release(struct inode *inode, struct file *filp)
{
    printk("\nrelease\n");
    return 0;
}


/*字符设备操作函数集*/
static struct file_operations  led_chr_dev_fops = 
{
    .owner = THIS_MODULE,
    .open = led_chr_dev_open,
    .release = led_chr_dev_release,
};



/*
*驱动初始化函数
*/
static int __init led_chrdev_init(void)
{
    int ret = 0;
    printk("led chrdev init\n");
    //第一步
    //采用动态分配的方式，获取设备编号，次设备号为0，
    //设备名称为EmbedCharDev，可通过命令cat  /proc/devices查看
    //DEV_CNT为1，当前只申请一个设备编号
    ret = alloc_chrdev_region(&led_devno, 0, DEV_CNT, DEV_NAME);
    if(ret < 0){
        printk("fail to alloc led_devno\n");
        goto alloc_err;
    }

    led_chrdev_class = class_create(THIS_MODULE, "led_chrdev");
    //第二步
    //关联字符设备结构体cdev与文件操作结构体file_operations
    cdev_init(&led_chr_dev, &led_chr_dev_fops);
    //第三步
    //添加设备至cdev_map散列表中
    ret = cdev_add(&led_chr_dev, led_devno, DEV_CNT);
    if(ret < 0)
    {
        printk("fail to add cdev\n");
        goto add_err;
    }

    //创建设备
    device_create(led_chrdev_class, NULL, led_devno, NULL,
			      DEV_NAME);
    return 0;

add_err:
    //添加设备失败时，需要注销设备号
    unregister_chrdev_region(led_devno, DEV_CNT);
alloc_err:
    return ret;
}





/*
*驱动注销函数
*/

static void __exit led_chrdev_exit(void)
{
    printk("chrdev exit\n");
   
    device_destroy(led_chrdev_class, led_devno);   //清除设备
    cdev_del(&led_chr_dev);                        //清除设备号
    unregister_chrdev_region(led_devno, DEV_CNT);  //取消注册字符设备
    class_destroy(led_chrdev_class);               //清除类
}


module_init(led_chrdev_init);
module_exit(led_chrdev_exit);

MODULE_LICENSE("GPL");

