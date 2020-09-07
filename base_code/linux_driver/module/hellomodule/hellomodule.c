#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int __init hello_init(void)
 {
    printk(KERN_EMERG "[ KERN_EMERG ]  Hello  Module Init\n");
    printk( "[ default ]  Hello  Module Init\n");
 return 0;
}

static void __exit hello_exit(void)
{
    printk("[ default ]   Hello  Module Exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

//MODULE_LICENSE("GPL2");
MODULE_AUTHOR("embedfire ");
MODULE_DESCRIPTION("hello world module");
MODULE_ALIAS("test_module");