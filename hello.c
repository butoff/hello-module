#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define COUNT   4
#define NAME    "hello"

MODULE_LICENSE("GPL");

static int major;
static int minor;

static __init int hello_init(void)
{
    dev_t dev;
    int res;

    res = alloc_chrdev_region(&dev, 0, COUNT, NAME);
    if (res != 0) {
        printk(KERN_INFO "can't alloc chrdev region\n");
        goto exit;
    } else {
        major = MAJOR(dev);
        minor = MINOR(dev);
        printk("allocated %d:%d\n", major, minor);
    }

exit:
    return 0;
}

static __exit void hello_exit(void)
{
    if (major != 0) {
        printk(KERN_INFO "unregister %d:%d\n", major, minor);
        unregister_chrdev_region(MKDEV(major, minor), COUNT);
    } else {
        printk(KERN_INFO "chrdev region wasn't allocated, nothing to unregister\n");
    }
}

module_init(hello_init);
module_exit(hello_exit);
