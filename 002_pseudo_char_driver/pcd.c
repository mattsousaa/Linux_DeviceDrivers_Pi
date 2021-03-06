#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h> 	/* for MAJOR and MINOR */
#include <linux/uaccess.h>	/* for copy_to_user and copy_from_user */

#define DEV_MEM_SIZE 512

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__ /* print function name used by pr_info */

/* pseudo device's memory */
char device_buffer[DEV_MEM_SIZE];

/* this holds the device number */
dev_t device_number;

/* Cdev variable */
struct cdev pcd_cdev;

/*holds the class pointer */
struct class *class_pcd;

/*holds the device pointer */
struct device *device_pcd;

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence){

	loff_t temp;

	pr_info("lseek requested \n");
	pr_info("Current value of the file position = %lld \n", filp->f_pos);

	switch(whence){
		case SEEK_SET:
			if((offset > DEV_MEM_SIZE) || (offset < 0))
				return -EINVAL;		/* whence is not valid */
			filp->f_pos = offset;
		break;

		case SEEK_CUR:
			temp = filp->f_pos + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;		/* whence is not valid */	
			filp->f_pos = temp;
		break;

		case SEEK_END:
			temp = DEV_MEM_SIZE + offset;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;		/* whence is not valid */	
			filp->f_pos = temp;
		break;
	
		default:
			return -EINVAL; 		/* whence is not valid */
	}

	pr_info("New value of the file position = %lld \n", filp->f_pos);

	/* Return newly updated file position */
    return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){

	pr_info("Read requested for %zu bytes \n", count);
	pr_info("Current file position = %lld \n", *f_pos); /* loff_t is a typedef for long long int (signed data) */

	/*1. Adjust the 'count' */
	if((*f_pos + count) > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;

	/*2. Copy to user */
	if(copy_to_user(buff, &device_buffer[*f_pos], count)){
		/* See include/uapi/asm-generic/errno-base.h file */
		return -EFAULT; /* Return bad address */
	}

	/*3. Update the current file position */
	*f_pos += count;

	pr_info("Number of bytes successfully read = %zu \n", count);
	pr_info("Updated file position = %lld \n", *f_pos);	
	
	/*4. Return number of bytes which have been successfully read */
    return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){

	pr_info("Write requested for %zu bytes \n", count);
	pr_info("Current file position = %lld \n", *f_pos); /* loff_t is a typedef for long long int (signed data) */

	/*1. Adjust the 'count' */
	if((*f_pos + count) > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;

	if(!count){
		pr_err("No space left on the device \n");
		return -ENOMEM;	/* Return out of memory */
	}

	/*2. Copy from user */
	if(copy_from_user(&device_buffer[*f_pos], buff, count)){
		/* See include/uapi/asm-generic/errno-base.h file */
		return -EFAULT; /* Return bad address */
	}

	/*3. Update the current file position */
	*f_pos += count;

	pr_info("Number of bytes successfully written = %zu \n", count);
	pr_info("Updated file position = %lld \n", *f_pos);	
	
	/*4. Return number of bytes which have been successfully written */
    return count;
}

int pcd_open(struct inode *inode, struct file *filp){
	pr_info("Open was successful \n");
    return 0;
}

int pcd_release(struct inode *inode, struct file *filp){
	pr_info("Release was successful \n");
    return 0;
}

/* file operations of the driver 
   C99 method using designated initializers */
struct file_operations pcd_fops = {
	.open = pcd_open,
	.write = pcd_write,
	.read = pcd_read,
	.llseek = pcd_lseek,
	.release = pcd_release,
	.owner = THIS_MODULE
};

static int __init pcd_driver_init(void){

	int ret; /* return value for error check */

	/* 1. Dynamically allocate a device number 
	 * Returns zero or a negative error code */
	ret = alloc_chrdev_region(&device_number, 0, 1, "pcd_devices");

	/* Checks if an error has occurs on alloc_chrdev_region function */
	if(ret < 0){
		pr_err("Alloc chrdev failed \n");
		goto out;
	}

	pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

	/* 2. Initialize the cdev structure with fops 
	 * This is a void function */
	cdev_init(&pcd_cdev, &pcd_fops); 

	/* 3. Register a device (cdev structure) with VFS */
	pcd_cdev.owner = THIS_MODULE;
	/* A negative error code is returned on failure */
	ret = cdev_add(&pcd_cdev, device_number, 1);

	/* Checks if an error has occurs on cdev_add function */
	if(ret < 0){
		pr_err("Cdev add failed \n");
		goto unreg_chrdev;
	}

	/* 4. Create device class under /sys/class/ 
	 * Returns &struct class pointer on success, or ERR_PTR() on error */
	class_pcd = class_create(THIS_MODULE, "pcd_class");

	/* Checks if an error has occurs on class_create function
	 * ERR_PTR: Converts error code (int) to pointer */
	if(IS_ERR(class_pcd)){
		pr_err("Class creation failed \n");		
		ret = PTR_ERR(class_pcd);			/* PTR_ERR: Converts pointer to error code(int) */
		goto cdev_del;
	}

	/*5. Populate the sysfs with device information 
	 * Returns &struct class pointer on success, or ERR_PTR() on error */
	device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");

	/* Checks if an error has occurs on device_create function
	 * ERR_PTR: Converts error code (int) to pointer */
	if(IS_ERR(device_pcd)){
		pr_err("Device create failed \n");
		ret = PTR_ERR(device_pcd);			/* PTR_ERR: Converts pointer to error code(int) */
		goto class_del;
	}

	pr_info("Module init was successful\n");
	return 0;

class_del:
	class_destroy(class_pcd);

cdev_del:
	cdev_del(&pcd_cdev);	

unreg_chrdev:
	unregister_chrdev_region(device_number, 1);

out: 
	pr_info("Module insertion failed \n");
	return ret;
}

static void __exit pcd_driver_cleanup(void){

	/* From botton up */
	device_destroy(class_pcd, device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("Module unloaded \n");

}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateus Sousa");
MODULE_DESCRIPTION("A pseudo character driver");
