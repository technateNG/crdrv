#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>

#define CRDRV_DEVICE_NAME "crdrv"
#define CRDRV_CLASS_NAME "cr"

#define RDCR4(st_cr4)  asm volatile( \
                          ".intel_syntax noprefix\n" \
                          "mov %0, cr4\n" \
                          ".att_syntax\n" \
                          : "=r"(st_cr4) \
                       )

#define WRCR4(st_cr4) asm volatile( \
                          ".intel_syntax noprefix\n" \
                          "mov cr4, %0\n" \
                          ".att_syntax\n" \
                          : \
                          : "r"(st_cr4) \
                      )

MODULE_LICENSE("GPL");
MODULE_AUTHOR("technateNG");
MODULE_DESCRIPTION("LKM to control an RDPMC scope.");
MODULE_VERSION("0.1");

enum IoctlCommands
{
    RDPMC_DISABLE = 0,
    RDPMC_ENABLE = 1
};

static int DrvOpen(struct inode* p_inode, struct file* p_file);
static int DrvRelease(struct inode* p_inode, struct file* p_file);
static ssize_t DrvRead(struct file* p_file, char* p_buffer, size_t st_len, loff_t* p_offset);
static ssize_t DrvWrite(struct file* p_file, const char* p_buffer, size_t st_len, loff_t* p_offset);
static long DrvIoctl(struct file* p_file, unsigned int i_ioctl_num, unsigned long l_ioctl_mem_ptr);

static int gi_major;

static const struct file_operations gs_fops = 
{
    .open = DrvOpen,
    .release = DrvRelease,
    .write = DrvWrite,
    .read = DrvRead,
    .unlocked_ioctl = DrvIoctl
};

static struct class*  gp_module_class  = NULL;
static struct device* gp_module_device = NULL;

static int DrvOpen(struct inode* p_inode, struct file* p_file)
{
    return 0;
}

static ssize_t DrvRead(struct file* p_file, char* p_buffer, size_t st_len, loff_t* p_offset)
{
    return 0;
}

static ssize_t DrvWrite(struct file* p_file, const char* p_buffer, size_t st_len, loff_t* p_offset)
{
    return 0;
}

static int DrvRelease(struct inode* p_inode, struct file* p_file)
{
    return 0;
}

static long DrvIoctl(struct file* p_file, unsigned int i_ioctl_num, unsigned long l_ioctl_mem_ptr)
{
    switch(i_ioctl_num)
    {
        case RDPMC_DISABLE:
        {
            size_t st_cr4;
            unsigned int ui_cpu = smp_processor_id();
            RDCR4(st_cr4);
            st_cr4 &= ~(1 << 8);
            WRCR4(st_cr4);
            printk(KERN_INFO "crdrv: RDPMC instruction disabled for CPU: %d\n", ui_cpu);
            return 0;
        }
        case RDPMC_ENABLE:
        {
            size_t st_cr4;
            unsigned int ui_cpu = smp_processor_id();
            RDCR4(st_cr4);
            st_cr4 |= (1 << 8);
            WRCR4(st_cr4);
            printk(KERN_INFO "crdrv: RDPMC instruction enabled for CPU: %d\n", ui_cpu);
            return 0;
        }
    }
    return 1;
}

static int __init DrvInit(void)
{
    gi_major = register_chrdev(0, CRDRV_DEVICE_NAME, &gs_fops);
    if (gi_major < 0)
    {
        printk(KERN_ALERT "crdrv: failed to register major number\n");
        return gi_major;
    }
    printk(KERN_INFO "crdrv: module loaded\n");
    gp_module_class = class_create(THIS_MODULE, CRDRV_CLASS_NAME);
    if (IS_ERR(gp_module_class))
    {
        unregister_chrdev(gi_major, CRDRV_DEVICE_NAME);
        printk(KERN_ALERT "crdrv: failed to register device class\n");
        return PTR_ERR(gp_module_class);
    }
    gp_module_device = device_create(gp_module_class, NULL, MKDEV(gi_major, 0), NULL, CRDRV_DEVICE_NAME);
    if (IS_ERR(gp_module_device))
    {
        class_destroy(gp_module_class);
        unregister_chrdev(gi_major, CRDRV_DEVICE_NAME);
        printk(KERN_ALERT "crdrv: failed to create the device\n");
        return PTR_ERR(gp_module_device);
    }
    printk(KERN_INFO "crdrv: device class created correctly\n");
    return 0;
}

static void __exit DrvExit(void)
{
    device_destroy(gp_module_class, MKDEV(gi_major, 0));
    class_destroy(gp_module_class);
    unregister_chrdev(gi_major, CRDRV_DEVICE_NAME);
    printk(KERN_INFO "crdrv: module unloaded\n");
}

module_init(DrvInit);
module_exit(DrvExit);
