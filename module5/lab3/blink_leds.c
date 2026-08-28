/*
 * Blink keyboard LEDs until the module is unloaded.
 */
#include <linux/console.h>
#include <linux/init.h>
#include <linux/kd.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/vt_kern.h>

#define BLINK_DELAY    (HZ / 5)
#define ALL_LEDS_ON    0x07
#define RESTORE_LEDS   0xFF

MODULE_DESCRIPTION("Example module illustrating the use of keyboard LEDs");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Popov");

static struct timer_list blink_timer;
static struct tty_struct *console_tty;
static struct kobject *leds_kobject;
static unsigned int led_mask;
static unsigned int led_status = RESTORE_LEDS;

static int set_keyboard_leds(unsigned int mask)
{
	return console_tty->driver->ops->ioctl(console_tty, KDSETLED, mask);
}

static void blink_timer_func(struct timer_list *timer)
{
	unsigned int mask = READ_ONCE(led_mask);

	led_status = led_status == mask ? RESTORE_LEDS : mask;
	set_keyboard_leds(led_status);
	mod_timer(timer, jiffies + BLINK_DELAY);
}

static ssize_t led_mask_show(struct kobject *kobj,
			     struct kobj_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "%u\n", READ_ONCE(led_mask));
}

static ssize_t led_mask_store(struct kobject *kobj,
			      struct kobj_attribute *attr,
			      const char *buf, size_t count)
{
	unsigned int mask;
	int error;

	error = kstrtouint(buf, 0, &mask);
	if (error)
		return error;
	if (mask > ALL_LEDS_ON)
		return -EINVAL;

	WRITE_ONCE(led_mask, mask);
	mod_timer(&blink_timer, jiffies + BLINK_DELAY);

	return count;
}

static struct kobj_attribute led_mask_attribute =
	__ATTR(test, 0660, led_mask_show, led_mask_store);

static int __init kbleds_init(void)
{
	struct tty_struct *tty;
	int error;

	console_lock();
	if (fg_console < 0 || fg_console >= MAX_NR_CONSOLES ||
	    !vc_cons[fg_console].d) {
		console_unlock();
		pr_err("kbleds: foreground console is unavailable\n");
		return -ENODEV;
	}

	tty = tty_kref_get(vc_cons[fg_console].d->port.tty);
	console_unlock();
	if (!tty || !tty->driver || !tty->driver->ops ||
	    !tty->driver->ops->ioctl) {
		pr_err("kbleds: foreground console has no usable TTY driver\n");
		tty_kref_put(tty);
		return -ENODEV;
	}

	console_tty = tty;
	timer_setup(&blink_timer, blink_timer_func, 0);

	leds_kobject = kobject_create_and_add("systest", kernel_kobj);
	if (!leds_kobject) {
		tty_kref_put(console_tty);
		return -ENOMEM;
	}

	error = sysfs_create_file(leds_kobject, &led_mask_attribute.attr);
	if (error) {
		pr_err("kbleds: failed to create /sys/kernel/systest/test: %d\n",
		       error);
		kobject_put(leds_kobject);
		tty_kref_put(console_tty);
		return error;
	}

	pr_info("kbleds: loaded; write an LED mask from 0 to 7 to "
		"/sys/kernel/systest/test\n");
	return 0;
}

static void __exit kbleds_exit(void)
{
	/* Stop new sysfs writes before shutting down the self-rearming timer. */
	kobject_put(leds_kobject);
	timer_shutdown_sync(&blink_timer);
	set_keyboard_leds(RESTORE_LEDS);
	tty_kref_put(console_tty);
	pr_info("kbleds: unloaded\n");
}

module_init(kbleds_init);
module_exit(kbleds_exit);
