#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h> 	/* for MAJOR and MINOR */
#include <linux/uaccess.h>	/* for copy_to_user and copy_from_user */

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__ /* print function name used by pr_info */

#define NO_OF_DEVICES 4

#define MEM_SIZE_MAX_PCDEV1  1024
#define MEM_SIZE_MAX_PCDEV2  512
#define MEM_SIZE_MAX_PCDEV3  1024
#define MEM_SIZE_MAX_PCDEV4  512

/* permission codes */
#define RDONLY 0x01			/* Read only */
#define WRONLY 0x10			/* Write only */
#define RDWR   0x11			/* Read and Write */

/* pseudo device's memory */
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

/* Device private data structure */
struct pcdev_private_data {
	char *buffer;											/* Base address of pcdev */													
	unsigned size;											
	const char *serial_number;	
	int perm;												/* Permission variable */
	struct cdev cdev;										/* Cdev variable */
};

/* Driver private data structure */
struct pcdrv_private_data {
	int total_devices;
	dev_t device_number;									/* this holds the device number */
	struct class *class_pcd;								/* holds the class pointer */
	struct device *device_pcd;								/* holds the device pointer */
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv_data = {
	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		[0] = {
			.buffer = device_buffer_pcdev1, 
			.size = MEM_SIZE_MAX_PCDEV1,
			.serial_number = "PCDEV1XYZ123",
			.perm = RDONLY
		}, 

		[1] = {
			.buffer = device_buffer_pcdev2, 
			.size = MEM_SIZE_MAX_PCDEV2,
			.serial_number = "PCDEV2XYZ123",
			.perm = WRONLY 
		},

		[2] = {
			.buffer = device_buffer_pcdev3, 
			.size = MEM_SIZE_MAX_PCDEV3,
			.serial_number = "PCDEV3XYZ123",
			.perm = RDWR 
		},

		[3] = {
			.buffer = device_buffer_pcdev4, 
			.size = MEM_SIZE_MAX_PCDEV4,
			.serial_number = "PCDEV4XYZ123",
			.perm = RDWR 
		}
	}
};

loff_t pcd_lseek(struct file *filp, loff_t offset, int whence){

#if 0
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
#endif
	return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){

#if 0
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
#endif
	return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){

#if 0
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
#endif
	return -ENOMEM;
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
	int i;	 /* Count variable */

	/* 1. Dynamically allocate a device number 
	 * Returns zero or a negative error code */
	ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcdevs");

	/* Checks if an error has occurs on alloc_chrdev_region function */
	if(ret < 0){
		pr_err("Alloc chrdev failed \n");
		goto out;
	}

	/* 4. Create device class under /sys/class/ 
	 * Returns &struct class pointer on success, or ERR_PTR() on error 
	 * Do not put this function inside loop */
	pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");

	/* Checks if an error has occurs on class_create function
	 * ERR_PTR: Converts error code (int) to pointer */
	if(IS_ERR(pcdrv_data.class_pcd)){
		pr_err("Class creation failed \n");		
		ret = PTR_ERR(pcdrv_data.class_pcd);			/* PTR_ERR: Converts pointer to error code(int) */
		goto cdev_del;
	}

	for(i = 0; i < NO_OF_DEVICES; i++){
		pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(pcdrv_data.device_number+i), MINOR(pcdrv_data.device_number+i));
	
		/* 2. Initialize the cdev structure with fops 
	 	 * This is a void function */
		cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops); 

		/* 3. Register a device (cdev structure) with VFS */
		pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
		/* A negative error code is returned on failure */
		ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number+i, 1);

		/* Checks if an error has occurs on cdev_add function */
		if(ret < 0){
			pr_err("Cdev add failed \n");
			goto unreg_chrdev;
		}

		/* 5. Populate the sysfs with device information 
		 * Returns &struct class pointer on success, or ERR_PTR() on error */
		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number+i, NULL, "pcdev-%d", i+1);

		/* Checks if an error has occurs on device_create function
		 * ERR_PTR: Converts error code (int) to pointer */
		if(IS_ERR(pcdrv_data.device_pcd)){
			pr_err("Device create failed \n");
			ret = PTR_ERR(pcdrv_data.device_pcd);			/* PTR_ERR: Converts pointer to error code(int) */
			goto class_del;
		}
	}

	pr_info("Module init was successful\n");
	return 0;

class_del:
	class_destroy(pcdrv_data.class_pcd);

cdev_del:
	cdev_del(&pcdrv_data.pcdev_data[i].cdev);	

unreg_chrdev:
	unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);

out: 
	pr_info("Module insertion failed \n");
	return ret;

}

static void __exit pcd_driver_cleanup(void){

#if 0
	/* From botton up */
	device_destroy(class_pcd, device_number);
	class_destroy(class_pcd);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number, 1);
	pr_info("Module unloaded \n");
#endif 

}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mateus Sousa");
MODULE_DESCRIPTION("A pseudo character driver which handles n devices");
