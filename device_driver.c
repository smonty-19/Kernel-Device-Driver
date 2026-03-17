#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>  // Required for copy_to_user

#define DEVICE_NAME "pes_device"
static int major_number;
static char message[] = "Hello from the Kernel space!\n";
static int message_ptr;

static int dev_open(struct inode *inodep, struct file *filep) {
   message_ptr = 0; // Reset pointer when file is opened
   printk(KERN_INFO "PES Device: Opened\n");
   return 0;
}

// The Read function
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    int bytes_to_read = strlen(message) - message_ptr;

    if (bytes_to_read == 0) return 0; // EOF: Tell 'cat' we are done

    // copy_to_user(to, from, size) - The safe way to move data to user space
    error_count = copy_to_user(buffer, message + message_ptr, bytes_to_read);

    if (error_count == 0) {
        printk(KERN_INFO "PES Device: Sent %d characters to user\n", bytes_to_read);
        message_ptr += bytes_to_read;
        return bytes_to_read;
    } else {
        printk(KERN_INFO "PES Device: Failed to send characters\n");
        return -EFAULT; // Return a "Bad Address" error
    }
}

static int dev_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "PES Device: Closed\n");
   return 0;
}

static struct file_operations fops = {
   .open = dev_open,
   .read = dev_read,    // Link the new read function
   .release = dev_release,
};

static int __init pes_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) return major_number;
    printk(KERN_INFO "PES Device: Major number is %d\n", major_number);
    return 0;
}

static void __exit pes_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
}

module_init(pes_init);
module_exit(pes_exit);
MODULE_LICENSE("GPL");

