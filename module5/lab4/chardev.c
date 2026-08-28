#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/irq.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/poll.h>
#include <linux/uaccess.h>
#include "chardev.h" 
 
#define SUCCESS 0 
#define DEVICE_NAME "chardev"
#define BUF_LEN 80
#define MSG_BUF 256
 
static int major;
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
};
 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
static char msg[MSG_BUF];
static char user_data[BUF_LEN];
static struct class *cls; 

static struct file_operations chardev_fops = { 
    .read = device_read, 
    .write = device_write, 
    .open = device_open, 
    .release = device_release, 
}; 
 
static int __init chardev_init(void) { 
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops); 
 
    if (major < 0) { 
        pr_alert("Registering char device failed with %d\n", major); 
        return major; 
    } 
    pr_info("I was assigned major number %d.\n", major); 
 
    cls = class_create(DEVICE_NAME);

    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME); 
    pr_info("Device created on /dev/%s\n", DEVICE_NAME); 
    return SUCCESS; 
} 
 
static void __exit chardev_exit(void){ 
    device_destroy(cls, MKDEV(major, 0)); 
    class_destroy(cls); 
    unregister_chrdev(major, DEVICE_NAME); 
    pr_info("Device /dev/%s removed\n", DEVICE_NAME);
} 

static int device_open(struct inode *inode, struct file *file){ 
    static int counter = 0; 
 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    if (counter < 5){
        snprintf(msg, MSG_BUF, "Newcomer! You addressed to this device %d times already!\nContains:%s\n", counter++, user_data);
    } else {
        snprintf(msg, MSG_BUF, "Aren't tou tired to use this devices %d times in a row?\nContains:%s\n", counter++, user_data);
    } 
    try_module_get(THIS_MODULE); 
    return SUCCESS; 
} 
 
static int device_release(struct inode *inode, struct file *file){ 
    atomic_set(&already_open, CDEV_NOT_USED); 
    module_put(THIS_MODULE); 
    return SUCCESS; 
} 

static ssize_t device_read(struct file *filp, char __user *buffer,
                           size_t length, loff_t *offset) {
    int bytes_read = 0; 
    int msg_len = strlen(msg);

    if (*offset >= msg_len)
        return 0;

    if (length > msg_len - *offset)
        length = msg_len - *offset;

    if (copy_to_user(buffer, msg + *offset, length))
        return -EFAULT;

    *offset += length;
    bytes_read = length;
    
    return bytes_read; 
} 
 
static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off) { 
    size_t copy_len = len;

    if (copy_len > BUF_LEN - 1) {
        copy_len = BUF_LEN - 1;
    }

    memset(msg, 0, BUF_LEN);

    if (copy_from_user(user_data, buff, copy_len)) {
        return -EFAULT;
    }

    if (copy_len > 0 && user_data[copy_len - 1] == '\n') {
        user_data[copy_len - 1] = '\0';
    } 
    
    return len; 
} 
 
module_init(chardev_init); 
module_exit(chardev_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Popov");
MODULE_DESCRIPTION("Information exchange");