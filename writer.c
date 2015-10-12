#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <asm/param.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Zyazulkin");

static int write_count;
struct timer_list exp_timer;
static int active;
static int delay;

static void create_timer(void);

static ssize_t write_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", write_count);
}

static void print_hello_world(unsigned long data)
{
        int i;

        for (i = 0; i < data; ++i) {
            printk("Hello world! ");
        }

        printk("\n");
        create_timer();
}

static void create_timer()
{
    if (active) {
        del_timer(&exp_timer);
        active = 0;
    }

    if (write_count != 0) {
        exp_timer.expires = jiffies + delay * HZ;
        exp_timer.data = write_count;
        exp_timer.function = print_hello_world;
        active = 1;
        add_timer(&exp_timer);
    }
}

static ssize_t write_count_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%du", &write_count);
    create_timer();
	return count;
}

static struct kobj_attribute writer_attrb = __ATTR(writer_count, 0666, write_count_show, write_count_store);

static struct kobject *kobj;

static int __init writer_init(void)
{
	int ret;

    active = 0;
    delay = 3;
    init_timer_on_stack(&exp_timer);

	kobj = kobject_create_and_add("writer", NULL);
	if (!kobj) {
		return - ENOMEM;
    }

    ret = sysfs_create_file(kobj, &writer_attrb.attr);
 	if (ret) {
        kobject_put(kobj);
    	return ret;
    }

	return 0;
}

static void __exit writer_exit(void)
{
    if (active) {
        del_timer(&exp_timer);
    }

	sysfs_remove_file(kobj, &writer_attrb.attr);
	kobject_put(kobj);
}

module_init(writer_init);
module_exit(writer_exit);
