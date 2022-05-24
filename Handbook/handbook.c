#include <linux/fscrypt.h>
#include <linux/init.h>
#include <linux/limits.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Belyakov Stepan");


#define MAX_DEV 2
#define BUF_LEN 256

static int handbook_open(struct inode *inode, struct file *file);
static int handbook_release(struct inode *inode, struct file *file);
static long handbook_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t handbook_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t handbook_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations handbook_fops = {
    .owner      = THIS_MODULE,
    .open       = handbook_open,
    .release    = handbook_release,
    .unlocked_ioctl = handbook_ioctl,
    .read       = handbook_read,
    .write      = handbook_write
};

struct mychar_device_data {
    struct cdev cdev;
};

struct Handbook {
    char* second_name;
    char* other_data;
    struct Handbook* prev_user;
    struct Handbook* next_user;
};
static struct Handbook* USERS_HEAD = NULL;

static int dev_major = 0;
static struct class *handbook_class = NULL;
static struct mychar_device_data handbook_data;

static char Message[BUF_LEN];

static int handbook_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init handbook_init(void)
{
    dev_t dev;

    alloc_chrdev_region(&dev, 0, MAX_DEV, "handbook");

    dev_major = MAJOR(dev);

    handbook_class = class_create(THIS_MODULE, "handbook");
    handbook_class->dev_uevent = handbook_uevent;

    
    cdev_init(&handbook_data.cdev, &handbook_fops);
    handbook_data.cdev.owner = THIS_MODULE;

    cdev_add(&handbook_data.cdev, MKDEV(dev_major, 0), 1);

    device_create(handbook_class, NULL, MKDEV(dev_major, 0), NULL, "handbook");


    return 0;
}

static void __exit handbook_exit(void)
{
    struct Handbook* user = USERS_HEAD; 
    while (user != NULL) {
        kfree(user->second_name);
        kfree(user->other_data);
        user = user->next_user;
        kfree(user->prev_user);
    }

    device_destroy(handbook_class, MKDEV(dev_major, 0));

    class_unregister(handbook_class);
    class_destroy(handbook_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static int handbook_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int handbook_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long handbook_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static ssize_t handbook_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    /*
    printk("all users:\n");
    struct Handbook* all = USERS_HEAD;
    for (; all != NULL; all = all->next_user) {
        printk("%s\n", all->second_name);
    }
    */

    struct Handbook* user = USERS_HEAD;
    for (; user != NULL && 0 != strcmp(user->second_name, Message); user = user->next_user) {
    }


    if (user == NULL) {
        printk("person not found\n");
        return -EFAULT;
    }
    char data[BUF_LEN];
    char* data_ptr = data;
    strcpy(data_ptr, user->second_name);

    data_ptr[strlen(user->second_name)] = ' ';
    data_ptr[strlen(user->second_name) + 1] = 0;
    data_ptr = strcat(data_ptr, user->other_data);
    
    size_t datalen = strlen(data_ptr); 
    data_ptr[datalen++] = '\n';
    data_ptr[datalen] = 0;
    printk("Data from the user: %s\n", data_ptr);
     
    if (count > datalen) {
        count = datalen;
    }
    
    if (copy_to_user(buf, data_ptr, count)) {
        return -EFAULT;
    }

    return count;
}


static ssize_t handbook_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    
    char raw_data[BUF_LEN]; 
    char* raw_data_ptr = raw_data;


    size_t ncopied = copy_from_user(raw_data, buf, count);

    if (ncopied == 0) {
        printk("Copied %zd bytes from the user\n", count);
    } else {
        printk("Could't copy %zd bytes from the user\n", ncopied);
        return ncopied;
    }

    raw_data[count] = 0;
    
    //printk("Data from the user: %s,\n", raw_data);
    
    char* operation_name = strsep(&raw_data_ptr, " \n");
    char* second_name = strsep(&raw_data_ptr, " \n");

    if (0 == strcmp(operation_name, "get")) {
        strcpy(Message, second_name);
    } else if (0 == strcmp(operation_name, "add")) { 
        struct Handbook* user = USERS_HEAD;
        for (; user != NULL && 0 != strcmp(user->second_name, second_name); user = user->next_user) {
        }
        if (user != NULL) {
            printk("username exist\n");
            return -EFAULT;
        }   

        user = kmalloc(sizeof(struct Handbook), GFP_KERNEL);
        user->second_name = second_name;
        user->other_data = strsep(&raw_data_ptr, "\n");
        
        //user->number = strsep(&raw_data_ptr, " \n");
        //user->email = strsep(&raw_data_ptr, " \n");
        
        int data_len = strlen(user->second_name); 
        char* data = kmalloc(data_len, GFP_KERNEL);
        memcpy(data, user->second_name, data_len);
        user->second_name = data;
        
        data_len = strlen(user->other_data);
        data = kmalloc(data_len, GFP_KERNEL);
        memcpy(data, user->other_data, data_len);
        user->other_data = data;

        user->prev_user = NULL;
        user->next_user = USERS_HEAD;
        if (USERS_HEAD != NULL) {
            USERS_HEAD->prev_user = user;
        }
        USERS_HEAD = user;

    } else if (0 == strcmp(operation_name, "del")) {
        struct Handbook* user = USERS_HEAD;
        for (; user != NULL && 0 != strcmp(user->second_name, second_name); user = user->next_user) {
        }
        if (user == NULL) {
            printk("username doesnt exist\n");
            return -EFAULT;
        }
        if (user->prev_user != NULL) {
            user->prev_user->next_user = user->next_user;
        }
        if (user->next_user != NULL) {
            user->next_user->prev_user = user->prev_user;
        }
        if (user == USERS_HEAD) {
            USERS_HEAD = user->next_user;
        }

        kfree(user->second_name);
        kfree(user->other_data);
        kfree(user);
    
    }
    return count;        
    
}


module_init(handbook_init);
module_exit(handbook_exit);

