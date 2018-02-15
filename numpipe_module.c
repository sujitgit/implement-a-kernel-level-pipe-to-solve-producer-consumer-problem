/* Include all the necessary libraries here. */
#include <linux/init.h>      /* Contains macros such as __init and __exit. */
#include <linux/module.h>     /* Contains headers for the loadable kernel modules. */
#include <linux/kernel.h>     /* Contains types, macro and function of kernel. */
#include <linux/fs.h>         /* file operations data structure is defined*/
#include  <linux/semaphore.h>
#include  <linux/device.h>
#include  <linux/string.h>
#include  <asm/uaccess.h>
#include <linux/slab.h>
#define NAME_OF_DEVICE "numpipe"
#define CLASS_OF_DEVICE "CHARDEV"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SUJIT"); 
MODULE_DESCRIPTION("A pipe to solve consumer producer problem");
MODULE_VERSION("1.0"); 
int deviceNumber;
static struct class*  MyPipeClass  = NULL; 
static struct device* MyPipeDevice = NULL;  
//static struct timespec xtime;
//static struct timespec *gtime;
static int noOfUsersOnFile;
static size_t byteCounter;
char pipeStringReturn[500];
//function declarations for char device
static ssize_t device_read(struct file *file_pointer, char *buffer_pointer, size_t length_to_read, loff_t *offset_pointer);
static ssize_t device_write(struct file *file_pointer, const char *buffer_pointer, size_t length_to_read, loff_t *offset_pointer);
static int device_open(struct inode *inodep, struct file *file_pointer);
static int device_close(struct inode *inodep, struct file *file_pointer);

struct semaphore empty;       /* counts number of empty buffer slots */
struct semaphore full;     /* checks if buffer is full*/
struct semaphore mutex;          /* to handle critical region  */
static int writeCounter = 0;
int sizeOfBuffer = 0;
module_param(sizeOfBuffer, int, S_IRUSR | S_IWUSR);
//int maxLength = 100;
int maxLength = 1;
static struct file_operations file_op =
{
   .open = device_open,
   .read = device_read,
   .write = device_write,
   .release = device_close,
};

// init is called when module is installed- that is register, classify, and create the device
static int __init numpipe_init(void)
{
   deviceNumber=register_chrdev(0, NAME_OF_DEVICE, &file_op);
   if (deviceNumber<0){
      printk(KERN_ALERT "device number went wrong");
   } else {
   }  
   MyPipeClass = class_create(THIS_MODULE, CLASS_OF_DEVICE);
   if (IS_ERR(MyPipeClass)){ 
      printk(KERN_ALERT "class went wrong");
      return PTR_ERR(MyPipeClass);
   } else {
   }
   MyPipeDevice = device_create(MyPipeClass, NULL, MKDEV(deviceNumber, 0), NULL, NAME_OF_DEVICE);
   if (IS_ERR(MyPipeDevice)){
      printk(KERN_ALERT "Failed to create the device\n");
      class_destroy(MyPipeClass);
            unregister_chrdev(deviceNumber, NAME_OF_DEVICE);            
            return PTR_ERR(MyPipeDevice);
   }
   sema_init(&full, 0);
   sema_init(&empty, sizeOfBuffer);
   sema_init(&mutex, 1);
   return 0;
}



// called when module is removed
static void __exit numpipe_exit(void)
{
   device_destroy(MyPipeClass, MKDEV(deviceNumber, 0));    
      class_unregister(MyPipeClass);                        
      class_destroy(MyPipeClass);        
      unregister_chrdev(deviceNumber, NAME_OF_DEVICE);
   printk(KERN_ALERT "module ends\n");
}

static ssize_t device_read(struct file *file_pointer, char *buffer_pointer, size_t length_to_read, loff_t *offset_pointer){
   unsigned int noOfBytesRead=0, moveBufferCount, buff_counter;
   printk(KERN_ALERT "MyPipe READ:ENTERING-READ BLOCK IN CHAR DEVICE\n");
   byteCounter=0;
   //xtime = current_kernel_time();
   //gtime = kmalloc(sizeof(struct timespec),GFP_KERNEL);
   //getnstimeofday(gtime);
   //sprintf(pipeStringReturn,"%s %ld %ld %s %s %ld  %ld %s","current_kernel_time: ", xtime.tv_sec, xtime.tv_nsec, "\n", "getnstimeofday: ", gtime->tv_sec, gtime->tv_nsec, "\n");
   //printk(KERN_ALERT "\ncurrent_kernel_time: %s", pipeStringReturn);
   // copy_to_user : ( * to, *from, size) - returns 0 on success
   for(buff_counter=0; buff_counter < maxLength; buff_counter++) {
         if( down_interruptible(&full) < 0 ){
            printk(KERN_ALERT "Buffer empty");
            return -1;
         }
         if( down_interruptible(&mutex) < 0) { 
            printk(KERN_ALERT "Entering critical region");
         }
         byteCounter = copy_to_user(buffer_pointer+buff_counter, pipeStringReturn, 1);
         if (byteCounter != 0) {
            printk(KERN_ALERT "Error writing to user space\n");
            return -EFAULT;
         } else {
            printk(KERN_ALERT "Read operation has succeeded");
         }
         noOfBytesRead++;
         for(moveBufferCount=0;moveBufferCount < writeCounter-1; moveBufferCount++) {
            pipeStringReturn[moveBufferCount]=pipeStringReturn[moveBufferCount+1];
         }
         writeCounter--;
         up(&mutex);
         up(&empty);
   }
      return noOfBytesRead; 
   //byteCounter = copy_to_user(buffer_pointer, pipeStringReturn, strlen(pipeStringReturn));
   //if (byteCounter != 0) {
   // printk(KERN_ALERT "Failed to write %zu bytes into user space\n", byteCounter);
   // return -EFAULT;
   //} else {
   // printk(KERN_ALERT "Copying to user space has succeeded");
   //}
   //kfree(gtime);
}
static ssize_t device_write(struct file *file_pointer, const char *buffer_pointer, size_t length_to_read, loff_t *offset_pointer){
   int noOfBytesWritten=0, status=0, buff_counter=0;
   //int buff_counter=0;
   while(buff_counter < maxLength) {
      if( down_interruptible(&empty) < 0){
         printk(KERN_ALERT "down_interruptible empty check");
      }
      if( down_interruptible(&mutex) < 0){
         printk(KERN_ALERT "down_interruptible mutex check");
      }
      status = copy_from_user(pipeStringReturn+writeCounter, buffer_pointer+buff_counter, 1);
      if(status < 0){
         printk(KERN_ALERT "Error in copy_from_user");
         return -1;
      }
      noOfBytesWritten++;
      writeCounter++;
      buff_counter++; 
      up(&mutex); 
      up(&full);
   }
   return noOfBytesWritten;
}
static int device_open(struct inode *inodep, struct file *file_pointer){
   printk(KERN_ALERT "\nSTART OF OPEN BLOCK\n");
   noOfUsersOnFile++;
   printk(KERN_ALERT "MyPipe Open: Device has been opened by %d users\n", noOfUsersOnFile);
   return 0;
}
static int device_close(struct inode *inodep, struct file *file_pointer){
  noOfUsersOnFile--;
  printk(KERN_ALERT "MyPipe Release: The device has been opened by %d users\n", noOfUsersOnFile);
  return 0;
}

module_init(numpipe_init);
module_exit(numpipe_exit);
