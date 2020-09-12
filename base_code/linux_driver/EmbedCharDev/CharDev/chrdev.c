#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#define DEV_NAME            "EmbedCharDev"
#define DEV_CNT                 (1)
#define BUFF_SIZE               128
//定义字符设备的设备号
static dev_t devno;
//定义字符设备结构体chr_dev
static struct cdev chr_dev;
//数据缓冲区
static char vbuf[BUFF_SIZE];
static int chr_dev_open(struct inode *inode, struct file *filp);
static int chr_dev_release(struct inode *inode, struct file *filp);
static ssize_t chr_dev_write(struct file *filp, const char __user * buf, size_t count, loff_t *ppos);
static ssize_t chr_dev_read(struct file *filp, char __user * buf, size_t count, loff_t *ppos);
static struct file_operations  chr_dev_fops = 
{
    .owner = THIS_MODULE,
    .open = chr_dev_open,
    .release = chr_dev_release,
    .write = chr_dev_write,
    .read = chr_dev_read,
};

static int chr_dev_open(struct inode *inode, struct file *filp)
{
    printk("\nopen\n");
    return 0;
}

static int chr_dev_release(struct inode *inode, struct file *filp)
{
    printk("\nrelease\n");
    return 0;
}

static ssize_t chr_dev_write(struct file *filp, const char __user * buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret;
    int tmp = count ;
    if(p > BUFF_SIZE)
        return 0;
    if(tmp > BUFF_SIZE - p)
        tmp = BUFF_SIZE - p;
    ret = copy_from_user(vbuf, buf, tmp);
    *ppos += tmp;
    return tmp;
}

static ssize_t chr_dev_read(struct file *filp, char __user * buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret;
    int tmp = count ;
    static int i = 0;
    i++;
    if(p >= BUFF_SIZE)
        return 0;
    if(tmp > BUFF_SIZE - p)
        tmp = BUFF_SIZE - p;
    ret = copy_to_user(buf, vbuf+p, tmp);
    *ppos +=tmp;
    return tmp;
}

static int __init chrdev_init(void)
{
    int ret = 0;
    printk("chrdev init\n");
    //第一步
    //采用动态分配的方式，获取设备编号，次设备号为0，
    //设备名称为EmbedCharDev，可通过命令cat  /proc/devices查看
    //DEV_CNT为1，当前只申请一个设备编号
    ret = alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);
    if(ret < 0){
        printk("fail to alloc devno\n");
        goto alloc_err;
    }
    //第二步
    //关联字符设备结构体cdev与文件操作结构体file_operations
    cdev_init(&chr_dev, &chr_dev_fops);
    //第三步
    //添加设备至cdev_map散列表中
    ret = cdev_add(&chr_dev, devno, DEV_CNT);
    if(ret < 0)
    {
        printk("fail to add cdev\n");
        goto add_err;
    }
    return 0;

add_err:
    //添加设备失败时，需要注销设备号
    unregister_chrdev_region(devno, DEV_CNT);
alloc_err:
    return ret;
}
module_init(chrdev_init);

static void __exit chrdev_exit(void)
{
    printk("chrdev exit\n");
    unregister_chrdev_region(devno, DEV_CNT);

    cdev_del(&chr_dev);
}
module_exit(chrdev_exit);

MODULE_LICENSE("GPL");

