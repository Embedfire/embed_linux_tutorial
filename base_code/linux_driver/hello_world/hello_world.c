#include <linux/init.h>
#include <linux/module.h>

//默认不输出调试信息
//权限有限制
bool debug_on = 0;
module_param(debug_on, bool, S_IRUSR);


static int __init hello_init(void)
{
    if(debug_on)
        printk(KERN_ERR "[ DEBUG ] debug info output\n");
    printk("Hello World Module Init\n");
    return 0;
}
module_init(hello_init);


static void __exit hello_exit(void)
{
    printk("Hello World Module Exit\n");
}
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("embedfire");
MODULE_DESCRIPTION("hello world module");
MODULE_ALIAS("test_module");

