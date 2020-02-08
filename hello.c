#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static __init int hello_init(void)
{
    printk(KERN_INFO "hello kernel\n");
    return 0;
}

static __exit void hello_exit(void)
{
    printk(KERN_INFO "goodbye kernel\n");
}

module_init(hello_init);
module_exit(hello_exit);
