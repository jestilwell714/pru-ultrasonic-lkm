/**
 * @file   ebbchar.c
 * @author James
 * @date   12/06/2026
 * @version 0.1
 * @brief   An
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#include <linux/interrupt.h>
#include <linux/io.h>       
#include <linux/gpio.h>  
#include <asm/div64.h> 
#define  DEVICE_NAME "sensor"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "uss"        ///< The device class -- this is a character device driver
#define PRU_SHARED_RAM_PHYS_ADDR 0x4A310000 
#define PRU_SHARED_RAM_SIZE 0x3000


MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("James");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for the BBB");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  sensorClass  = NULL; ///< The device-driver class struct pointer
static struct device* sensorDevice = NULL;
static unsigned int gpio_interrupt = 71;
static unsigned int irq_number;
static void __iomem *pru_shared_mem_ptr;
static unsigned int shared_data_buffer;
static unsigned int distance_buffer;
static unsigned int num_sensors = 1;

static unsigned int history_window = 5;
static unsigned int history[history_window] = {0};
static int index = 0;
static unsigned int speed_of_sound = 343;
static struct kobject *sensor_kobj; 



// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .release = dev_release,
};


static ssize_t numSensorsShow(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", num_sensors);
}

static ssize_t numSensorsStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%u", &num_sensors);

    iowrite32(num_sensors, pru_shared_mem_ptr);
    return count;
}


static ssize_t speedSoundShow(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", speed_of_sound);
}

static ssize_t speedSoundStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%u", &speed_of_sound);
    return count;
}

static ssize_t historyWindowShow(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", history_window);
}

static ssize_t historyWindowStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%u", &history_window);
    iowrite32(history_window, pru_shared_mem_ptr+4);
    return count;
}


static struct kobj_attribute speed_of_sound_attr = __ATTR(speed_of_sound, 0664, speedSoundShow, speedSoundStore);
static struct kobj_attribute num_sensors_attr = __ATTR(num_sensors, 0664, numSensorsShow, numSensorsStore);
static struct kobj_attribute history_window_attr = __ATTR(history_window, 0664, historyWindowShow, historyWindowStore);

static struct attribute *sensor_attrs[] = {
    &num_sensors_attr.attr,
    &speed_of_sound_attr.attr,
    &history_window_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .name = NULL,
    .attrs = sensor_attrs,
};







static void calc_distance(struct tasklet_struct *t);
DECLARE_TASKLET(my_tasklet, calc_distance);

static irqreturn_t sensor_gpio_irq_handler(int irq,
void *dev_id) {
    // Top half:
    shared_data_buffer = ioread32(pru_shared_mem_ptr +8);

    // Declare bottom half
    tasklet_schedule(&my_tasklet);

    return IRQ_HANDLED;
}

static void calc_distance(struct tasklet_struct *t) {
    printk(KERN_INFO "SENSOR: Tasklet bottom-half is running!\n");

    history[index] = shared_data_buffer;
    unsigned int cycles = 0;
    ssize_t i;
    for(i = 0; i < history_window; ++i ) {
        cycles += history[i];
    }
    cycles /= history_window;

    unsigned int distance = (cycles * speed_of_sound)/4000000; // distance in cm

    index++;
    if (index >= history_window) {
        index = 0;
    }

    distance_buffer = distance;

    printk(KERN_INFO "SENSOR: Raw Shared Data: %u, Calculated Distance: %u cm\n", 
           shared_data_buffer, distance_buffer);
}

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init sensor_init(void){
    int result;
    printk(KERN_INFO "SENSOR: Initializing the SENSOR LKM\n");

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "SENSOR failed to register a major number\n");
      return majorNumber;
   }

   printk(KERN_INFO "SENSOR: registered correctly with major number %d\n", majorNumber);

    // Register the device class
   sensorClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(sensorClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "SENSOR: Failed to register device class\n");
      return PTR_ERR(sensorClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "SENSOR: device class registered correctly\n");

   // Register the device driver
   sensorDevice = device_create(sensorClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(sensorDevice)){               // Clean up if there is an error
      class_destroy(sensorClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "SENSOR: Failed to create the device\n");
      return PTR_ERR(sensorDevice);
   }
   printk(KERN_INFO "SENSOR: device class created correctly\n"); // Made it! device was initialized

    sensor_kobj = kobject_create_and_add("sensor",kernel_kobj->parent);



    printk(KERN_INFO "SENSOR: Kobject created correctly");

    
        result = sysfs_create_group(sensor_kobj, &attr_group);

    printk(KERN_INFO "SENSOR: associate attributes with that object");

    pru_shared_mem_ptr = ioremap(PRU_SHARED_RAM_PHYS_ADDR, PRU_SHARED_RAM_SIZE);
    printk(KERN_INFO "SENSOR: About to ioremap\n");
    if (!pru_shared_mem_ptr) {
        printk(KERN_ALERT "SENSOR: ioremap failed\n");
        return -ENOMEM;
    }

    if (!gpio_is_valid(gpio_interrupt)){
      printk(KERN_INFO "SENSOR: invalid LED GPIO\n");
      return -ENODEV;
   }
   if (gpio_request(gpio_interrupt, "sensor_gpio") != 0) {
        printk(KERN_ALERT "SENSOR: Failed to request GPIO %d\n", gpio_interrupt);
        return -EBUSY;
    }
    printk(KERN_INFO "SENSOR: About to request GPIO\n");
    gpio_direction_input(gpio_interrupt);
   irq_number = gpio_to_irq(gpio_interrupt); // map GPIO to IRQ number
    result = request_irq(irq_number,
        sensor_gpio_irq_handler,
        IRQF_TRIGGER_RISING,
        "sensor_handler",
        NULL);

    printk(KERN_INFO "SENSOR: Requested IRQ");

   return 0;
}


static void __exit sensor_exit(void) {
    device_destroy(sensorClass, MKDEV(majorNumber, 0));     // remove the device
    class_unregister(sensorClass);                          // unregister the device class
    class_destroy(sensorClass);                             // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
    iounmap(pru_shared_mem_ptr);
    free_irq(irq_number, NULL); 
    gpio_free(gpio_interrupt);
    tasklet_kill(&my_tasklet);
    kobject_put(sensor_kobj);
    printk(KERN_INFO "SENSOR: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "SENSOR: Device has been opened\n");
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, &distance_buffer, sizeof(distance_buffer));

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "SENSOR: Sent characters to the user\n");
      return sizeof(distance_buffer);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "SENSOR: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "SENSOR: Device successfully closed");
    return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(sensor_init);
module_exit(sensor_exit);