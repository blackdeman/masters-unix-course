#include <linux/init.h>		/* __init and __exit macroses */
#include <linux/kernel.h>	/* KERN_INFO macros */
#include <linux/module.h>	/* required for all kernel modules */
#include <linux/moduleparam.h>	/* module_param() and MODULE_PARM_DESC() */

#include <linux/fs.h>		/* struct file_operations, struct file */
#include <linux/miscdevice.h>	/* struct miscdevice and misc_[de]register() */
#include <linux/mutex.h>	/* mutexes */
#include <linux/uaccess.h>	/* copy_{to,from}_user() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Demchenko <blackdeman@gmail.com>");
MODULE_DESCRIPTION("/dev/null with limited capacity");

static unsigned long capacity = 0;
module_param(capacity, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(capacity, "Internal buffer capacity");

static unsigned long used = 0;
static struct mutex lock;

static ssize_t read_null(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t write_nulll(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	ssize_t result = count;
	if (mutex_lock_interruptible(&lock)) {
		result = -ERESTARTSYS;
		goto out;
	}

	used += count;
	
	if (capacity && used > capacity) {
		used = capacity;
		result = -ENOSPC;	
	}

	mutex_unlock(&lock);
out:
	return result;
}

static ssize_t ioctl_nulll(struct file *file, unsigned int cmd, unsigned long arg)
{
	ssize_t result = 0;
	int ret;
	if (cmd == BLKGETSIZE64) {
		if (mutex_lock_interruptible(&lock)) {
			result = -ERESTARTSYS;
			goto out;
		}
		ret = copy_to_user((void __user *)arg, &used, sizeof(used));
		if (ret) {
			printk(KERN_ERR "copy_to_user error: didn't copy %d bytes\n", ret);
			result = -EFAULT;		
		}
		mutex_unlock(&lock);
	}
out:        
	return result;
}

static const struct file_operations nulll_fops = {
	.owner = THIS_MODULE,
	.read		= read_null,
	.write		= write_nulll,
	.unlocked_ioctl = (void*)&ioctl_nulll,
};

static struct miscdevice nulll_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "nulll",
	.fops = &nulll_fops
};

static int __init nulll_init(void)
{
	if (capacity < 0) {
		printk(KERN_ERR "nulll device doesn't accept negative capacity \n");
	}

	mutex_init(&lock);
	misc_register(&nulll_misc_device);

	if (capacity)
		printk(KERN_INFO "nulll device has been registered, capacity is %lu bytes\n", capacity);
	else {
		printk(KERN_INFO "nulll device has been registered, capacity is infinite\n");		
	}
	return 0;
}

static void __exit nulll_exit(void)
{
	misc_deregister(&nulll_misc_device);
	printk(KERN_INFO "nulll device has been unregistered\n");

	mutex_destroy(&lock);
}

module_init(nulll_init);
module_exit(nulll_exit);