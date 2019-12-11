#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

static int foo;
static char baz[32];
static char bar;


//the buf used in the below callbacks represents the kernel buffer
static ssize_t foo_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
        dump_stack();
        ret= sprintf(buf, "%d", foo);
        printk("data string is %s\n", buf);
        return ret;
       
}


static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{

        dump_stack();
	sscanf(buf, "%du", &foo);
	printk("data  is %d\n", foo);
	return count;
}

 
static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0644, foo_show, foo_store);

static ssize_t b_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	int var;
        dump_stack();

	if (strcmp(attr->attr.name, "baz") == 0)
		var = baz;
	else
		var = bar;
	return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	char var[count];
        dump_stack();

	sscanf(buf, "%du", &var);
	if (strcmp(attr->attr.name, "baz") == 0)
		baz = var;
	else
		bar = var;
	return count;
}

//setting up attributes
static struct kobj_attribute baz_attribute = __ATTR(baz, 0600, b_show, b_store);

static struct kobj_attribute bar_attribute = __ATTR(bar, 0666, b_show, b_store);

//creating a group so that the creation and destruction of the group can be done all at once
static struct attribute *attrs[] = {
	&foo_attribute.attr,
	&baz_attribute.attr,
	&bar_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *example_kobj

static int __init example_init(void)
{
	int retval=0;

	//the following API is used to create a directory name as kobject_example under the directrory 
	//pointed by the kernel_obj
	example_kobj = kobject_create_and_add("kobject_example", kernel_kobj);
	//here,the example_kobj is pointer to the newly created system object or kobject
	

	if (!example_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(example_kobj, &attr_group);
return retval;
}

static void __exit example_exit(void)
{

	kobject_put(example_kobj);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com>");
	
