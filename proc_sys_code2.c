//procfs code to create a directory and add the required infomation or devices instance

//header files required for the module
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include <linux/list.h>

/*static struct _getter_struct{
	struct list_head list;
	char info[25];
};*/

static LIST_HEAD(getter_list);		//initialising getter_list & head of the list as 'getter_list'

//following are the entries to be made inside the procfs
static struct proc_dir_entry *entry1 = NULL;
static struct proc_dir_entry *entry2 = NULL;

//read sequence for the module

//start for the sequence 
/*static void * getter_seq_start(struct seq_file *file,loff_t *pos){
	struct _getter_struct *struct_ptr;//struct_ptr points to the current struct{} obj in the list
					//whenever the offset is equal to the current position
					//the current struct{} object ptr is returned


	loff_t offset=0;		//start iterating from the 0th position in the list of 
					//struct{} object

	list_for_each_entry(struct_ptr,&getter_list,list){	//iterates over the list of struct{} objects
		if(pos==offset++){
			printk("the start sequence initiated\n");
			return struct_ptr;		//return a ptr to the current struct{} object
		}
	}
	printk("start sequence terminated\n");
	return NULL;
}*/
//the above start sequence is not used because it has the struct getter{} object 
//but, for the current problem we need to place taskstruct{} inside the list
//since, we want the info regarding the process

//start sequence 
static void * getter_seq_start(struct seq_file *file,loff_t *pos){
	//it points to the current process 
	struct task_struct *process_ptr;
	
	loff_t offset = 0;
	for_each_process(process_ptr){		//the process_ptr is set to the current process using 
						//the offset and the current position given by 
						//the vfs filesystem
		if(pos==offset++){
			printk("start sequence successfully initiated\n");
			return process_ptr;
		}
	}
	printk("the start sequence got terminated\n");
	return NULL;
}
//next for the sequence
static void* getter_seq_next(struct seq_file *file,void *v,loff_t *pos){
	struct list_head *list_ptr = ((struct task_struct *)v)->tasks.next;	//sets the list head to point to the 
      								//next task/process pointed by the
								//current process
	++*pos;

	printk("the next sequence initiated:  %d\n",*pos);
	
	return (list_ptr != &getter_list)?list_entry(list_ptr,struct task_struct,tasks):NULL;
	//tasks ??	
}
//show for the sequence
static int getter_seq_show(struct seq_file *file,void *process_ptr){	//the file pts to the current file
								//process_ptr is pointing to the current
								//task which is invoking the module
	
	const struct task_struct *process = process_ptr;	//task_struct is a predefined struct having
								//all the members related to the info of the
								//current process or user process
	
	char *kernel_buffer = kmalloc(256,GFP_KERNEL);		//allocate some memory to put the data
								//info_str acts as a buffer
								//it gets the string which will be build
								//it is a kernel buffer
	printk("Inside the show sequence\n");
	
	sprintf(kernel_buffer,"pid  = %d\ntgid = %d\n",process->pid,process->tgid);
	//putting the data in the kernel buffer 
       	seq_printf(file,kernel_buffer);
	printk(KERN_INFO "the retrun value of seq_printf\n");
	kfree(kernel_buffer);
	return 0;
}

//stop for the sequence
static void getter_seq_stop(struct seq_file *file,void *v){
	printk("sequence is terminating\n");
}

//sequence operation table
static struct seq_operations getter_seq_ops={
	.start	= getter_seq_start,
	.next	= getter_seq_next,
	.show	= getter_seq_show,
	.stop	= getter_seq_stop,	
};

//open sequence for the module
static int getter_seq_open(struct inode *inode, struct file *file){
	printk("Inside the open sequence\n");
	dump_stack();
	return seq_open(file,&getter_seq_ops);
}

//creating a file operation table for our module directory 
static struct file_operations getter_proc_fops={
	.owner = THIS_MODULE,
	.open  = getter_seq_open,
	.read  = seq_read,
};

//init call-back of the module
static int getter_init(void){
	
//	struct _getter_struct *getter_object;		//this represents the module in the kernel space

	//creating directories for the module 
	entry1 = proc_mkdir("check",NULL);
	entry2 = proc_create("get_pid_tgid",S_IRUSR,entry1,&getter_proc_fops);
	
	//check if the proc_create is successful
	if(entry2==NULL)
		return -EINVAL;
	printk("this is INIT call-back of the module\n");
	return 0;
}

//exit callback for cleanup of the module
static void getter_clean(void){
	remove_proc_entry("get_pid_tgid",entry1);
	remove_proc_entry("check",NULL);
	printk("getter cleanup is done\n");
}

module_init(getter_init);
module_exit(getter_clean);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Akshay Ramteke <akid04387@gmail.com>");
