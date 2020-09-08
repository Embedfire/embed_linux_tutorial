#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int itype=0;
module_param(itype,int,0);

static bool btype=0;
module_param(btype,bool,0700);

static char ctype=0;
module_param(ctype,byte,0);

static char  *stype=0;
module_param(stype,charp,0644);

static int __init param_init(void)
 {
 printk(KERN_ALERT "param init!\n");
 printk(KERN_ALERT "itype=%d\n",itype);
 printk(KERN_ALERT "btype=%d\n",btype);
 printk(KERN_ALERT "ctype=%d\n",ctype);
 printk(KERN_ALERT "stype=%s\n",stype);
 return 0;
}

static void __exit param_exit(void)
{
 printk(KERN_ALERT "module exit!\n");
}

EXPORT_SYMBOL(itype);

 int my_add(int a, int b)
  {
    return a+b;
 }

EXPORT_SYMBOL(my_add);

int my_sub(int a, int b)
 {
    return a-b;
 }

EXPORT_SYMBOL(my_sub);

module_init(param_init);
module_exit(param_exit);

MODULE_LICENSE("GPL2");
MODULE_AUTHOR("embedfire ");
MODULE_DESCRIPTION("module_param");
MODULE_ALIAS("module_param");