
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // required for various structures related to files liked fops. 
#include <asm/uaccess.h> // required for copy_from and copy_to user functions 
#include <linux/semaphore.h>
#include <linux/cdev.h> 

// sonradan eklendi
// http://www.linuxforu.com/2011/04/character-device-files-creation-operations/
#include <linux/kdev_t.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/device.h>

/*
static struct cdev c_dev; // Global variable for the character device structure
*/

static struct class *cl; // Global variable for the device class

static int Major;
static int written, read;

// Device pointer
static struct cdev *kernel_cdev; 

static dev_t dev_no;

struct mydevice {
  char array[100];
  struct semaphore sem;
} myDevice;

int open(struct inode *inode, struct file *filp)
{
 
 printk(KERN_INFO "Inside open \n");
 
 if(down_interruptible(&myDevice.sem)) {
   printk(KERN_INFO " could not hold semaphore");
   return -1;
 }
 
 read = written;
 return 0;
 
}

int release(struct inode *inode, struct file *filp) {
  printk (KERN_INFO "Inside close \n");
  printk(KERN_INFO "Releasing semaphore");
  up(&myDevice.sem);
  return 0;
}

ssize_t read_char(struct file *filp, char *buff, size_t count, loff_t *offp) {
  unsigned long ret;
  printk("Inside read \n");
  if(count > read) 
    count = read;
  read = read -count;
  ret = copy_to_user(buff, myDevice.array, count);
  return count;
}

ssize_t write_char(struct file *filp, const char *buff, size_t count, loff_t *offp) { 
  unsigned long ret; 
  printk(KERN_INFO "Inside write \n");
  ret = copy_from_user(myDevice.array, buff, count); 
  written += count;
  return count;
}

struct file_operations fops = {
  owner:  THIS_MODULE,
  read:  read_char,
  write:  write_char,
  open:   open,
  release: release
};


int myDevice_init (void) {
  
  int ret;
  
  kernel_cdev = cdev_alloc(); 
  kernel_cdev->ops = &fops;
  kernel_cdev->owner = THIS_MODULE;
  printk (" Inside init module\n");
  ret = alloc_chrdev_region( &dev_no , 0, 1,"myDevice_dev");
    
  Major = MAJOR(dev_no);
  printk (KERN_INFO" The major number for your device is %d\n", Major); 
  
  if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
  {
    unregister_chrdev_region(dev_no, 1);
    return -1;
  }
  
  if (device_create(cl, NULL, dev_no, NULL, "mynull") == NULL)
  {
    class_destroy(cl);
    unregister_chrdev_region(dev_no, 1);
    return -1;
  }  

  // Not this
  // init_MUTEX(&myDevice.sem);
  sema_init(&myDevice.sem, 1);
  cdev_init(kernel_cdev, &fops);

  ret = cdev_add( kernel_cdev, dev_no,1);
  if(ret < 0 ) 
  {
    device_destroy(cl, dev_no);
    class_destroy(cl);
    
    unregister_chrdev_region(dev_no, 1);
    
    printk(KERN_INFO "Unable to allocate cdev");
    return ret;
  }

  return 0;
}

void myDevice_cleanup(void) {
  printk(KERN_INFO " Inside cleanup_module\n");
  cdev_del(kernel_cdev);
  unregister_chrdev_region(Major, 1);
  
  device_destroy(cl, dev_no);
  class_destroy(cl);
  unregister_chrdev_region(dev_no, 1);
  
  
}

MODULE_LICENSE("GPL"); 
module_init(myDevice_init);
module_exit(myDevice_cleanup);