#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>

#define COUNT   4
#define NAME    "hello"

MODULE_LICENSE("GPL");

static int major;
static int minor;

struct hello_dev {
    /* struct hello_qset *data; */
    int quantum;
    int qset;
    unsigned long size;
    unsigned int access_key;
    struct semaphore sem;
    struct cdev cdev;
};

static struct hello_dev hello_devices[COUNT];

#define DATA_SIZE   0x10

static char data[0x100];
static int fully_read = 0;
static int fully_written = 0;

loff_t hello_llseek(struct file *filp, loff_t off, int i)
{
    return 0;
}

ssize_t hello_read(struct file *filp, char * __user cp, size_t size, loff_t *off)
{
    if (!fully_read) {
        raw_copy_to_user(cp, data, 6);
        fully_read = 1;
        return 6;
    } else {
        fully_read = 0;
        return 0;
    }
}

ssize_t hello_write(struct file *filp, const char * __user cp, size_t size, loff_t *off)
{
    int written = size < DATA_SIZE ? size : DATA_SIZE;
    raw_copy_from_user(data, cp, written);
    return size;
}

int hello_open(struct inode *inode, struct file *filp)
{
    struct hello_dev *dev;

    printk(KERN_INFO "open hello file");

    dev = container_of(inode->i_cdev, struct hello_dev, cdev);
    filp->private_data = dev;

    /* trim the length if write-only */
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        /*hello_trim(dev);*/
    }
    return 0;
}

int hello_release(struct inode *node, struct file *filp)
{
    return 0;
}

struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .llseek = hello_llseek,
    .read = hello_read,
    .write = hello_write,
/*    .ioctl = hello_ioctl, */
    .open = hello_open,
    .release = hello_release,
};

static void hello_setup_cdev(struct hello_dev *dev, int index)
{
    int err, devno = MKDEV(major, minor + index);
    
    cdev_init(&dev->cdev, &hello_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &hello_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    /* fail gracefully if need be */
    if (err) printk(KERN_NOTICE "Error %d adding hello%d", err, index);
}

static __init int hello_init(void)
{
    dev_t dev;
    int i, err;

    err = alloc_chrdev_region(&dev, 0, COUNT, NAME);
    if (err != 0) {
        printk(KERN_INFO "can't alloc chrdev region\n");
        goto exit;
    } else {
        major = MAJOR(dev);
        minor = MINOR(dev);
        printk("allocated %d:%d\n", major, minor);
    }

    for (i = 0; i < COUNT; i++) {
        hello_setup_cdev(&hello_devices[i], i);
    }

exit:
    return 0;
}

static __exit void hello_exit(void)
{
    int i;

    if (major != 0) {
        printk(KERN_INFO "unregister %d:%d\n", major, minor);
        unregister_chrdev_region(MKDEV(major, minor), COUNT);
    } else {
        printk(KERN_INFO "chrdev region wasn't allocated, nothing to unregister\n");
        return;
    }

    for (i = 0; i < COUNT; i++) {
        cdev_del(&hello_devices[i].cdev);
    }
}

module_init(hello_init);
module_exit(hello_exit);
