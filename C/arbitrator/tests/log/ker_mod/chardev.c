/* chardev.c 
 * Copyright (C) 1998-1999 by Ori Pomerantz
 * 
 * Create a character device (read only)
 */

/* The necessary header files */

/* Standard in kernel modules */
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */

#if 0
/* Deal with CONFIG_MODVERSIONS */
#if CONFIG_MODVERSIONS==1
#define MODVERSIONS
#include <linux/modversions.h>
#endif        
#endif

/* For character devices */
#include <linux/fs.h>       /* The character device 
                             * definitions are here */
#include <linux/wrapper.h>  /* A wrapper which does 
                             * next to nothing at
                             * at present, but may 
                             * help for compatibility
                             * with future versions 
                             * of Linux */


/* In 2.2.3 /usr/include/linux/version.h includes 
 * a macro for this, but 2.0.35 doesn't - so I add 
 * it here if necessary. */
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif


/* Conditional compilation. LINUX_VERSION_CODE is 
 * the code (as per KERNEL_VERSION) of this version. */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,2,0)
#include <asm/uaccess.h>  /* for put_user */
#endif

                             

#define SUCCESS 0


/* Device Declarations **************************** */

/* The name for our device, as it will appear 
 * in /proc/devices */
#define DEVICE_NAME "char_dev"


/* The maximum length of the message from the device */
#define BUF_LEN 80

/* Is the device open right now? Used to prevent 
 * concurent access into the same device */
static int Device_Open = 0;

/* The message the device will give when asked */
static char Message[BUF_LEN];

/* How far did the process reading the message 
 * get? Useful if the message is larger than the size 
 * of the buffer we get to fill in device_read. */
static char *Message_Ptr;

double mydouble;

/* This function is called whenever a process 
 * attempts to open the device file */
static int device_open(struct inode *inode, 
                       struct file *file)
{
  static int counter = 0;

#ifdef DEBUG
  printk ("device_open(%p,%p)\n", inode, file);
#endif

  /* This is how you get the minor device number in 
   * case you have more than one physical device using 
   * the driver. */
  printk("Device: %d.%d\n", 
         inode->i_rdev >> 8, inode->i_rdev & 0xFF);

  /* We don't want to talk to two processes at the 
   * same time */
  if (Device_Open)
    return -EBUSY;

  /* If this was a process, we would have had to be 
   * more careful here.
   *
   * In the case of processes, the danger would be 
   * that one process might have check Device_Open 
   * and then be replaced by the schedualer by another 
   * process which runs this function. Then, when the 
   * first process was back on the CPU, it would assume 
   * the device is still not open. 
   *
   * However, Linux guarantees that a process won't be 
   * replaced while it is running in kernel context. 
   *
   * In the case of SMP, one CPU might increment 
   * Device_Open while another CPU is here, right after 
   * the check. However, in version 2.0 of the 
   * kernel this is not a problem because there's a lock 
   * to guarantee only one CPU will be kernel module at 
   * the same time. This is bad in  terms of 
   * performance, so version 2.2 changed it. 
   * Unfortunately, I don't have access to an SMP box 
   * to check how it works with SMP.
   */

  Device_Open++;

  /* Initialize the message. */
  sprintf(Message, 
    "If I told you once, I told you %d times - %s",
    counter++,
    "Hello, world\n");
  /* The only reason we're allowed to do this sprintf 
   * is because the maximum length of the message 
   * (assuming 32 bit integers - up to 10 digits
   * with the minus sign) is less than BUF_LEN, which 
   * is 80. BE CAREFUL NOT TO OVERFLOW BUFFERS, 
   * ESPECIALLY IN THE KERNEL!!! 
   */ 

  Message_Ptr = (char*)&mydouble;

  /* Make sure that the module isn't removed while 
   * the file is open by incrementing the usage count 
   * (the number of opened references to the module, if 
   * it's not zero rmmod will fail)
   */
  MOD_INC_USE_COUNT;

  return SUCCESS;
}


/* This function is called when a process closes the 
 * device file. It doesn't have a return value in 
 * version 2.0.x because it can't fail (you must ALWAYS
 * be able to close a device). In version 2.2.x it is 
 * allowed to fail - but we won't let it. 
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
static int device_release(struct inode *inode, 
                          struct file *file)
#else 
static void device_release(struct inode *inode, 
                           struct file *file)
#endif
{
#ifdef DEBUG
  printk ("device_release(%p,%p)\n", inode, file);
#endif
 
  /* We're now ready for our next caller */
  Device_Open --;

  /* Decrement the usage count, otherwise once you 
   * opened the file you'll never get rid of the module.
   */
  MOD_DEC_USE_COUNT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
  return 0;
#endif
}


/* This function is called whenever a process which 
 * have already opened the device file attempts to 
 * read from it. */

extern double ___ieee754_log(double value);

static ssize_t device_read(struct file *file,
    char *buffer,    /* The buffer to fill with data */
    size_t length,   /* The length of the buffer */
    loff_t *offset)  /* Our offset in the file */
{
  /* Number of bytes actually written to the buffer */
  int bytes_read = 0;

  /* If we're at the end of the message, return 0 
   * (which signifies end of file) */
  //if (*Message_Ptr == 0)
  //  return 0;

  /* Actually put the data into the buffer */
  while (length)
  {
  while (length && Message_Ptr < (((char*)&mydouble)+sizeof(mydouble)))  {

    /* Because the buffer is in the user data segment, 
     * not the kernel data segment, assignment wouldn't 
     * work. Instead, we have to use put_user which 
     * copies data from the kernel data segment to the 
     * user data segment. */
    put_user(*(Message_Ptr++), buffer++);


    length --;
    bytes_read ++;
  }
  Message_Ptr = (char*)&mydouble;
  }

#ifdef DEBUG
   printk ("Read %d bytes, %d left\n",
     bytes_read, length);
#endif

   /* Read functions are supposed to return the number 
    * of bytes actually inserted into the buffer */
  return bytes_read;
}




/* This function is called when somebody tries to write 
 * into our device file - unsupported in this example. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
static ssize_t device_write(struct file *file,
    const char *buffer,    /* The buffer */
    size_t length,   /* The length of the buffer */
    loff_t *offset)  /* Our offset in the file */
#else
static int device_write(struct inode *inode,
                        struct file *file,
                        const char *buffer,
                        int length)
#endif
{
  printk("Device Write!\n");
  if (length >= sizeof(mydouble))
  {
      printk("%.2x|%.2x|%.2x|%.2x|%.2x|%.2x|%.2x|%.2x", 
              (int)buffer[0],
              (int)buffer[1],
              (int)buffer[2],
              (int)buffer[3],
              (int)buffer[4],
              (int)buffer[5],
              (int)buffer[6],
              (int)buffer[7]
              );
              
      mydouble = ___ieee754_log(*(double *)buffer);
  }
  return 0;
}




/* Module Declarations ***************************** */

/* The major device number for the device. This is 
 * global (well, static, which in this context is global
 * within this file) because it has to be accessible 
 * both for registration and for release. */
static int Major;

/* This structure will hold the functions to be 
 * called when a process does something to the device 
 * we created. Since a pointer to this structure is 
 * kept in the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions. */


struct file_operations Fops = {
  NULL,
  NULL,   /* seek */
  device_read, 
  device_write,
  NULL,   /* readdir */
  NULL,   /* select */
  NULL,   /* ioctl */
  NULL,   /* mmap */
  device_open,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
  NULL,   /* flush */
#endif
  device_release  /* a.k.a. close */
};


/* Initialize the module - Register the character device */
int init_module()
{
  /* Register the character device (atleast try) */
  Major = register_chrdev(0, 
                                 DEVICE_NAME,
                                 &Fops);

  /* Negative values signify an error */
  if (Major < 0) {
    printk ("%s device failed with %d\n",
            "Sorry, registering the character",
            Major);
    return Major;
  }

  printk ("%s The major device number is %d.\n",
          "Registeration is a success.",
          Major);
  printk ("If you want to talk to the device driver,\n");
  printk ("you'll have to create a device file. \n");
  printk ("We suggest you use:\n");
  printk ("mknod <name> c %d <minor>\n", Major);
  printk ("You can try different minor numbers %s",
          "and see what happens.\n");

  return 0;
}


/* Cleanup - unregister the appropriate file from /proc */
void cleanup_module()
{
  int ret;

  /* Unregister the device */
  ret = unregister_chrdev(Major, DEVICE_NAME);
 
  /* If there's an error, report it */ 
  if (ret < 0)
    printk("Error in unregister_chrdev: %d\n", ret);
}

