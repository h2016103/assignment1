/*
 *   - Create a random number generator module
 */

#include <linux/kernel.h>       /* We're doing kernel work */
#include <linux/module.h>       /* Specifically, a module */
#include <linux/fs.h>
#include <asm/uaccess.h>        /* for get_user and put_user */
#include <linux/ioctl.h>
#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/device.h>
#include <linux/cdev.h>

static dev_t first; // variable for device number
static struct cdev c_dev; // variable for the character device structure
static struct class *cls;
#define DEVICE_FILE_NAME "trng_dev"
#define MAJOR_NUM 'r'
#define SUCCESS 0
#define DEVICE_NAME "trng_dev"
#define BUF_LEN 80
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)


int file_read(struct file* f) {
   
    unsigned char ch_buf[7];  // char buffer to read no. from file
    int cpu_freq= 0, loop_var,temp_num;  


    mm_segment_t oldfs;  // to store current file segment 
    char __user *p = (__force char __user *)&ch_buf[0];
         loff_t offset = 0; // to set offset to read file
    oldfs = get_fs(); // to store old file segment 
    set_fs(get_ds());  // set the new segment 
    __vfs_read(f, p, 7, &offset);  // to read the file and store in ch buffer 
   
for(loop_var=0; ch_buf[loop_var]!=10 &&loop_var<7 ;loop_var++)
{
temp_num = ch_buf[loop_var]; // store ASCII value of numbers 
temp_num = temp_num - 48;  // convert ASCII value of number into number

cpu_freq = cpu_freq*10 + temp_num ;
}
    set_fs(oldfs); // again set the file segment into old one
    return cpu_freq;   // return num read from file
}   


void file_close(struct file* file) {
    filp_close(file, NULL);
}


struct file* file_open(const char* path, int flags,int mode) 
{
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags , mode); // return file pointer of current log file
    set_fs(oldfs);
    if(IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}















long device_ioctl(struct file *file,             /* to receive ioctl call from user space */
                  unsigned int ioctl_num,        /* number and param for ioctl */
                  unsigned long ioctl_param)
{
   
    int trand_num , cpu_freq , range_upper, range_lower, norm_freq , diff_range;

    struct file* fptr; /*to read  file in user space */

    /*
     * Switch according to the ioctl called
     */
    switch (ioctl_num) {
    case IOCTL_SET_MSG:
       
         break;

    case IOCTL_GET_MSG:
        
        break;

    case IOCTL_GET_NTH_BYTE:  /*using only this call */
       

         fptr = file_open("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", 0, O_RDWR);//getting pointer for curr freq log file    
         cpu_freq = file_read(fptr); /* read current freq from log file*/
         file_close(fptr);       /* close the file pointer */
         
         get_user(range_lower, (int *)ioctl_param + 0); // get lower range value from the user space 
         get_user(range_upper, (int *)ioctl_param + 1); // get upper range value from the user space
         
         diff_range = range_upper - range_lower; 
        
         norm_freq = cpu_freq% diff_range ; // quantized the random no. generated 
         
         trand_num = range_lower + norm_freq + 1 ;  // calculate the random no.
         put_user(trand_num, (int *)ioctl_param + 2);  // storing the random no. in the user space 
         
         
        return 0;
        break;
    }

    return SUCCESS;
}


struct file_operations f_ops = {
       
        .unlocked_ioctl = device_ioctl, // link the ioctl call to system call
      
};

/*
 * Initialize the module - Register the character device
 */
static int __init mychar_init(void) 
{
	//printk(KERN_INFO "Namaste: mychar driver registered");
	
	// STEP 1 : reserve <major, minor>
	if (alloc_chrdev_region(&first, 0, 1, "BITS-PILANI") < 0)
	{
		return -1;
	}
	
	// STEP 2 : dynamically create device node in /dev directory
    if ((cls = class_create(THIS_MODULE, "chrdev")) == NULL)
	{
		unregister_chrdev_region(first, 1);
		return -1;
	}
    if (device_create(cls, NULL, first, NULL, "trng_dev") == NULL)
	{
		class_destroy(cls);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	
	// STEP 3 : Link fops and cdev to device node
    cdev_init(&c_dev, &f_ops);
    if (cdev_add(&c_dev, first, 1) == -1)
	{
		device_destroy(cls, first);
		class_destroy(cls);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	return 0;
}
 
static void __exit mychar_exit(void) 
{
	cdev_del(&c_dev);
	device_destroy(cls, first);
	class_destroy(cls);
	unregister_chrdev_region(first, 1);
	printk(KERN_ALERT "random number driver unregistered\n\n");
}
module_init(mychar_init);
module_exit(mychar_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ABHINAV GARG");
MODULE_DESCRIPTION("random number driver");

