//device driver's model code {kset sample}


//headers/libraries required
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>


//container object or object for device instance

struct device_instance{
	struct kobject dev_kobj;	//it repersents the device instance in the kernel space
					//and is used to register the device instance to the 
					//sysfs/device model file system
	
	//following are the device instance specific attributes
	int dev_attr1;
	int dev_attr2;
	int dev_attr3;
};

//the following is a container for device instance specific attribute

struct dev_inst_attr{
	struct attribute attr;		//an attribute object used to manage the device instance 
						//specific attribute
	
	//following are the device specific instance's attribute's callbacks/methods
	ssize_t (*show)(struct device_instance *device,struct dev_inst_attr *attr,char *buf);
	ssize_t (*store)(struct device_instance *device,struct dev_inst_attr *attr,const char *buf,size_t count);
};

//the following are container_of macros used to get the corresponding structure related to the current
//object passed as a parameter 

#define get_device_instance(x) container_of(x,struct device_instance,dev_kobj);	//for device instance
#define get_dev_inst_attr(x) container_of(x,struct dev_inst_attr,attr);	//for device instance attribute

//following are the callbacks used by the sysfs operations
//here, we need to get the device instance from its kobject 
//and its device instance specific attributes from its attribute
//this can be done using the container_of macros defined initially
//after the above then call the show/store functions
//it returns the setup for the system object, its attributes & attributes specific callbacks

static ssize_t link_sys_dev_show(struct kobject *kobj,struct attribute *attr,char *buf){
	//the parameters are passed to the API by the sysfs for the setup
	
	//define two pointers to the device instance and device instance attribute
	struct device_instance *device;
	struct dev_inst_attr *dev_attr;
	
	//transpose to the device container & its attributes using the macros
	device = get_device_instance(kobj);
	dev_attr = get_dev_inst_attr(attr);

	//check if the correct device is accessed
	if(!dev_attr->show)	return -EIO;

	return dev_attr->show(device,dev_attr,buf);
}

//the following is for the store similar to the show as above it is also setup when the store SCAPI 
//is accessed in the user space code

static ssize_t link_sys_dev_store(struct kobject *kobj,struct attribute *attr,const char *buf,size_t count){
	
	//defining the pointer to have the corresponding setup
	struct device_instance *device;
	struct dev_inst_attr *dev_attr;

	device = get_device_instance(kobj);
	dev_attr = get_dev_inst_attr(attr);

	//check for errors
	if(!dev_attr->store)	return -EIO;

	return dev_attr->store(device,dev_attr,buf,count);
}

//creating the operation table for the sysfs operations to link 
//the above callbacks to the sysfs/device model

static struct sysfs_ops dev_sys_ops = {
	.show = link_sys_dev_show,
       	.store = link_sys_dev_store,	
};

//the following is release callback to free the memory held by our device related kobjects
//this is required by the framework

static void device_release(struct kobject *kobj){
	//to remove the device we need to get the device_instance related to the kobject
	//this is again done using the macros
	struct device_instance *device;
	device = get_device_instance(kobj);

	//kfree is a slab allocators api which can be used to remove the allocated memory
	kfree(device);
}

//following are the device specific callbacks which will be accessed further after the above setup
//of the device_instance and its attributes are complete the device will further invoke the following
//callbacks as per the framework

static ssize_t dev_attr_show(struct device_instance *device,struct dev_inst_attr *dev_attr,char *buf){
	//these parameters are passed to the callback by the link_sys_dev_show()
	
	if(strcmp(dev_attr->attr.name,"dev_attr1")==0)
		return sprintf(buf,"%d\n",device->dev_attr1);
	else if(strcmp(dev_attr->attr.name,"dev_attr2")==0)
		return sprintf(buf,"%d\n",device->dev_attr2);
	else
		return sprintf(buf,"%d\n",device->dev_attr3);
}

static ssize_t dev_attr_store(struct device_instance *device, struct dev_inst_attr *dev_attr,const char *buf, size_t count){
	//copying the data into the private variable from the buffer and then accessing it
	int parameter;

	sscanf(buf,"%du",parameter);
	
	if(strcmp(dev_attr->attr.name,"dev_attr1")==0)
		device->dev_attr1 = parameter;
	else if(strcmp(dev_attr->attr.name,"dev_attr2")==0)
		device->dev_attr2 = parameter;
	else
		device->dev_attr3 = parameter;
		
	return count;
}

//following is for associating the attributes to their attributes objects
//this is part of the framework

static struct dev_inst_attr assoc_dev_attr1 = __ATTR(dev_attr1,0644,dev_attr_show,dev_attr_store);
static struct dev_inst_attr assoc_dev_attr2 = __ATTR(dev_attr2,0644,dev_attr_show,dev_attr_store);
static struct dev_inst_attr assoc_dev_attr3 = __ATTR(dev_attr3,0644,dev_attr_show,dev_attr_store);

//grouping the attributes together 
//this is done so that the attributes can be created as well as destroyed 
//all at once

static struct attribute *dev_attr_group[] = {
	&assoc_dev_attr1.attr,
	&assoc_dev_attr2.attr,
	&assoc_dev_attr3.attr,
	NULL,
};

//now, for kset we have to initialise the kobj_type accordingly
//as it will link our kset and its kobjects and related device specific attributes 
//to the user space via several layers of vfs and system interface layer

static struct kobj_type device_type = {
	.sysfs_ops = &dev_sys_ops,
	.release = device_release,
	.default_attrs = dev_attr_group,
};


//the following pointer are initialised to setup the kset and its various device instances
static struct kset *kset_ptr;
static struct device_instance *device1;
static struct device_instance *device2;
static struct device_instance *device3;

//the following callback/method is used to create a device instance 
//it is used by the init method to create multiple device instances
static struct device_instance *create_device(const char *dev_name){
	struct device_instance *device;
	int ret;

	device = kzalloc(sizeof(struct device_instance),GFP_KERNEL);
	if(!device)	return NULL;

	//associating the kobject of the current device instance to the kset 
	//this kset, will be initialised in the init method of the module
	
	device->dev_kobj.kset = kset_ptr;

	//initialise and add the current device specific kobj to the kernel
	//here the parent need not be specified as we have already linked it to the kset
	//also the registration is done by the following api
	
	ret = kobject_init_and_add(&device->dev_kobj,&device_type,NULL,"%s",dev_name);
	if(ret){
		kobject_put(&device->dev_kobj);
		return NULL;
	}
	//this is to inform about the created kobject
	kobject_uevent(&device->dev_kobj,KOBJ_ADD);
	return device;
}

//this is to destroy the created device related objects
static void destroy_device_instance(struct device_instance *device){
	kobject_put(&device->dev_kobj);
}

//the modules init method/callback
static int device_init(void){
	
	kset_ptr = kset_create_and_add("kset_devices",NULL,kernel_kobj);
	if(!kset_ptr)	return -ENOMEM;

	device1 = create_device("device_1");
	if(!device1){
		destroy_device_instance(device1);
		return -EINVAL;
	}
	device2 = create_device("device_2");
	if(!device2){
		destroy_device_instance(device2);
		return -EINVAL;
	}
	device3 = create_device("device_3");
	if(!device3){
		destroy_device_instance(device3);
		return -EINVAL;
	}
	return 0;
}

//the modules exit method
static void device_exit(void){
	destroy_device_instance(device1);
	destroy_device_instance(device2);
	destroy_device_instance(device3);
	kset_unregister(kset_ptr);
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Akshay Ramteke");
