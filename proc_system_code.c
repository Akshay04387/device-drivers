//this is the system space part code for the procfs


//following are the kernel header files to be included 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/seq_file.h>

//the following structure manages the loaded kernel module in the system space

struct _mydriver_struct {	//the '_' is used before structures name denoting the type as kernel object 
	
	struct list_head list;	//doubly linked list having prev and next as its members and used to 
				//traverse overs such kernel objects in system space
				//this is given/provided as part of the frame-work and should be used 
				//wherever required
	
	char info[13];		//depends on the module and is info to be passed via the procfs files		
	char info1[10];
};

//the above used list needs to be initialised certain ways to initialise the list below is one of them
static LIST_HEAD(mydriver_list);

//creating handles to the directories
//originally, these are pointers to the proc_dir_entry{} objects
//these are initialised as NULL
static struct proc_dir_entry *entry1 =NULL;
static struct proc_dir_entry *entry2 =NULL;

//open operation method for the driver
static int mydriver_seq_open(struct inode *inode,struct file *file){
	printk("Inside the open callback for our driver\n");
	dump_stack();
	return seq_open(file,&mydriver_operation);
}


//OPERATIONS TABLE for the driver
//following are the operations mentioned for our module 
//for this we need to use the object of "file_operations"
//here, "mydriver_seq_open" method will be mentioned above the operations table
static struct file_operations mydriver_operation = {
	.owner	=THIS_MODULE,	.open	=mydriver_seq_open,	.read	=seq_read				
};

static int mydriver_init(void){

	struct _mydriver_struct *mydriver_obj_ptr;
		
	dump_stack();		//used for diagnostic messages during debugging

	entry1 =proc_mkdir("proc_test",NULL);	//create a proc_test directory inside the proc directory 
						//hence,the P2 is kept NULL
	entry2 =proc_create("Readme_info",S_IRUSR,entry1,&mydriver_operation);
	
	//check the proc_create is successful or the module will case FATAL problems
	if(entry2 == NULL)	
		return -EINVAL;
	for(int i=0;i<=10;i++){
		//memory allocation using slab allocator for our driver object
		//here, kzalloc is used to allocate memory 
		struct _mydriver_struct mydriver_object = kzalloc(sizeof(struct _mydriver_struct),GFP_KERNEL);
		//writing into the particular files object
		sprintf(mydriver_object->info,"NODE Number is: %d\n",i);

		//loading the structure into the list {this is part of the framework}
		//the list is for the particular file and is defined in the framework
		//we just have to add over structures object to the list using the following
		list_add_tail(&mydriver_object->list,&mydriver_list); 
	}
	return 0;	
}

static void mydriver_exit(void){
	struct _mydriver_struct *p,*n;
	list_for_each_entry_safe(p,n,&mydriver_list,list){
		kfree(p); 	
	}
	remove_proc_entry("Readme_info",entry1);
	remove_proc_entry("proc_test",NULL);
	printk("mydriver has exited\n");
}

module_init(mydriver_init);
module_exit(mydriver_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Akshay Ramteke");


