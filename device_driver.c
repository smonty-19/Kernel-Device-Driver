#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>  // Crucial for copy_to_user and copy_from_user

#define DEVICE_NAME "Monty_Device"
#define BUFFER_SIZE 1024

static int major_number;
static char kernel_buffer[BUFFER_SIZE]; 
static short size_of_message;

static int dev_open(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "Monty_Device: Device has been opened\n");
   return 0;
}

// READ: The kernel sends data to the user (cat /dev/Monty_Device)
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;

    // If offset is > 0, it means we already finished reading (EOF)
    if (*offset > 0) return 0;

    // copy_to_user(destination_in_user_space, source_in_kernel, size)
    error_count = copy_to_user(buffer, kernel_buffer, size_of_message);

    if (error_count == 0) {
        printk(KERN_INFO "Monty_Device: Sent %d characters to the user\n", size_of_message);
        *offset += size_of_message; 
        return size_of_message;
    } else {
        printk(KERN_INFO "Monty_Device: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }
}

// WRITE: The user sends data to the kernel (echo "hi" > /dev/Monty_Device)
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    // Basic overflow protection
    int to_copy = (len > BUFFER_SIZE) ? BUFFER_SIZE : len;

    // copy_from_user(destination_in_kernel, source_in_user_space, size)
    unsigned long not_copied = copy_from_user(kernel_buffer, buffer, to_copy);

    size_of_message = to_copy - not_copied;
    printk(KERN_INFO "Monty_Device: Received %d characters from the user\n", size_of_message);

    return size_of_message;
}

static int dev_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "Monty_Device: Device successfully closed\n");
   return 0;
}

static struct file_operations fops = {
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int __init monty_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Monty_Device failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "Monty_Device: registered with major number %d\n", major_number);
    return 0;
}

static void __exit monty_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Monty_Device: Goodbye from the Kernel!\n");
}

module_init(monty_init);
module_exit(monty_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("smonty-19");
MODULE_DESCRIPTION("A simple Linux character driver for Monty_Device");