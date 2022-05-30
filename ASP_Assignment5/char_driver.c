#include<linux/module.h>
#include <linux/fs.h>		
#include <linux/uaccess.h>	
#include <linux/init.h>		
#include <linux/slab.h>		
#include <linux/cdev.h>		
#include<linux/device.h>
#include<linux/mutex.h>
#include <linux/sem.h>
#define my_drv "my_drv"
#define ramdisk_size (size_t)16*PAGE_SIZE
#define my_magic_no 'Z'
#define ASP_CLEAR_BUF _IOW(my_magic_no,1,int)
dev_t first;
int dev_nr=3;
int majornumber=0;
static int cbt=0;
module_param(dev_nr,int,S_IRUGO);



struct asp_mycdev{
	struct cdev dev;
	char * ramdisk;
	struct semaphore sem;
	dev_t devNo;
	size_t file_size;
};

struct asp_mycdev* mcoll;

static struct class * mclass;

static int mopen(struct inode* inode, struct file* file){
	struct asp_mycdev *dtemp;
	dtemp = container_of(inode->i_cdev,struct asp_mycdev,dev);
	file->private_data=dtemp;
	return 0;
}

static ssize_t mread(struct file* filepointer,char __user* buf,size_t lbuf,loff_t *position){
	int nobytes;
        struct asp_mycdev* tdev=filepointer->private_data;
	pr_alert("Reading now");	
	if((lbuf+*position)>ramdisk_size){
		pr_info("Offset has exceeded");
		return 0;
	}
	if(down_interruptible(&(tdev->sem))){
		pr_info("Semaphore has been failed to acquire");
		return -ERESTARTSYS;
	}
	nobytes=lbuf-copy_to_user(buf,(tdev->ramdisk)+*position,lbuf);
	*position+=nobytes;
	up(&(tdev->sem));
	cbt=nobytes;
	return nobytes;
}

static ssize_t mwrite(struct file* filepointer, const char __user* buf,size_t lbuf,loff_t* position){
	int nobytes;
        struct asp_mycdev* tdev=filepointer->private_data;
	pr_alert("Writing now\n");	
	if((lbuf+*position)>ramdisk_size){
		pr_info("Offset has been exceeded");
		return 0;
	}
	if(down_interruptible(&(tdev->sem))){
		pr_info("Semaphore has been failed to acquire");
		return -ERESTARTSYS;
	}
	nobytes=lbuf-copy_from_user(tdev->ramdisk+*position,buf,lbuf);
	*position+=nobytes;
	up(&(tdev->sem));
	pr_alert("%d\n",nobytes);
	cbt=nobytes;	
	return nobytes;
	
}
static int mclose(struct inode *inode, struct file *file)
{
	pr_info(" Closing the device:\n\n");
	return 0;
}
static loff_t mlseek(struct file* file, loff_t offsetposition,int k){
	loff_t positionofoffset;
	struct asp_mycdev* tdev=file->private_data;
	switch(k){
		case SEEK_SET:
			positionofoffset=offsetposition;
			break;
		case SEEK_CUR:
			positionofoffset=file->f_pos+offsetposition;
			break;
		case SEEK_END:
			positionofoffset=cbt+offsetposition;
			break;
		default:
			return -EINVAL;
	}
	positionofoffset=positionofoffset>=0?positionofoffset:0;
	file->f_pos=positionofoffset;
	pr_info("seek successful\n");
	return positionofoffset;
}
static long mioctl(struct file* file, unsigned int cmd,unsigned long arg){
        struct asp_mycdev* tdev=file->private_data;	
        if(cmd==ASP_CLEAR_BUF){
		if(down_interruptible(&(tdev->sem))<0){
			pr_info("lock acquire failed\n");
			return -1;
		}
		memset(tdev->ramdisk,0,ramdisk_size);
		up(&(tdev->sem));
		file->f_pos=0;
		return 0;
	}
	else{
		return -EINVAL;
	}
}
static const struct file_operations moperations={
	.owner=THIS_MODULE,
	.open=mopen,
	.read=mread,
	.write=mwrite,
	.llseek=mlseek,
	.release=mclose,
	.unlocked_ioctl=mioctl
};
static void cdev_set_up(struct asp_mycdev* temp){
	cdev_init(&temp->dev,&moperations);
	if(cdev_add(&temp->dev,temp->devNo,1)<0){
		pr_alert("cdev add failed");
		kfree(temp->ramdisk);
		unregister_chrdev_region(temp->devNo,1);
		return;
	}
}
static int __init my_init(void){
	int i;	
	struct asp_mycdev* dev_arr=kmalloc(dev_nr*sizeof(struct asp_mycdev),GFP_KERNEL);
	mcoll=dev_arr;
	if(alloc_chrdev_region(&first,0,dev_nr,my_drv)<0){
		pr_alert("coulnt register major number");
		return ENODEV;
	}
	majornumber=MAJOR(first);
	for(i=0;i<dev_nr;i++){
		dev_arr[i].devNo=MKDEV(majornumber,i);
		dev_arr[i].ramdisk=kmalloc(ramdisk_size,GFP_KERNEL);
		sema_init(&dev_arr[i].sem,1);
		dev_arr[i].file_size=sizeof(dev_arr[i].ramdisk);		
		cdev_set_up(&dev_arr[i]);
	}
	mclass=class_create(THIS_MODULE,"other_class2");
	for(i=0;i<dev_nr;i++){
		device_create(mclass,NULL,dev_arr[i].devNo,NULL,"mycdev%d",i);
	}
	pr_info("\nmodule loaded....\n");
	return 0;
}

static void __exit my_exit(void){
	int i;
	for(i=0;i<dev_nr;i++){
		pr_info("error handle 1\n");
		kfree(mcoll[i].ramdisk);
		pr_info("error handle 2\n");
                device_destroy(mclass,mcoll[i].devNo);
		pr_info("error handle 3\n");		
                cdev_del(&mcoll[i].dev);
		pr_info("error handle 4\n");
		unregister_chrdev_region(mcoll[i].devNo,1);
		pr_info("error handle 5\n");
		
	}
	kfree(mcoll);
	pr_info("error handle 6\n");
        class_destroy(mclass);
        pr_info("module_unloaded");
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohit PS");
