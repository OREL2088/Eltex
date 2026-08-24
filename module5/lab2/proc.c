#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define PROC_NAME "proc"
#define PROC_BUFF 10
 
static int len,temp;
static char *msg;
static const int mode = 0666;
 
static ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp ) {
    if (count > temp)
        count = temp;

    if (copy_to_user(buf, msg + (len - temp), count))
        return -1;

    temp -= count;

    if (count == 0)
        temp = len;

    return count;
}
 
static ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp) {
    size_t bytes_to_copy = min(count, (size_t)PROC_BUFF);
    if (copy_from_user(msg, buf, bytes_to_copy))
        return -1;
    len = bytes_to_copy;
    temp = len;
    return bytes_to_copy;
}
 
static const struct proc_ops proc_fops = {
    proc_read: read_proc,
    proc_write: write_proc,
};
 
static int create_new_proc_entry(void) {
    msg = kmalloc(PROC_BUFF * sizeof(char), GFP_KERNEL);
    if (!msg)
        return -1;

    if (!proc_create(PROC_NAME, mode, NULL, &proc_fops)) {
        kfree(msg);
        msg = NULL;
        return -1;
    }

    return 0;
}
 
static int proc_init (void) {
    create_new_proc_entry();
    return 0;
}
 
static void proc_cleanup(void) {
    remove_proc_entry(PROC_NAME, NULL);
    kfree(msg);
}
 
MODULE_LICENSE("PVE");
MODULE_AUTHOR("Popov");
MODULE_DESCRIPTION("A simple module for proc test");
module_init(proc_init);
module_exit(proc_cleanup);
