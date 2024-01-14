#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/cdev.h>


#define PROC_DIR "ensea"
#define PROC_FILE_SPEED "ensea/speed"
#define PROC_FILE_DIRECTION "ensea/dir"


// Prototypes
static int leds_probe(struct platform_device *pdev);
static int leds_remove(struct platform_device *pdev);
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset);

static int create_proc_files(struct ensea_leds_dev *dev)

static int proc_read_speed(char *page, char **start, off_t offset, int count, int *eof, void *data);
static int proc_write_speed(struct file *file, const char *buffer, unsigned long count, void *data);

static int proc_read_dir(char *page, char **start, off_t offset, int count, int *eof, void *data);
static int proc_write_dir(struct file *file, const char *buffer, unsigned long count, void *data);

static void chenillard_timer_handler(unsigned long data);

static void cleanup_ensea_leds_class(void)

// An instance of this structure will be created for every ensea_led IP in the system
struct ensea_leds_dev {
    struct miscdevice miscdev;
    void __iomem *regs;
    u8 leds_value;
    int speed;
    u8 pattern;
    int direction;  
};

static struct class *ensea_leds_class;
static struct cdev ensea_leds_cdev;

static void cleanup_ensea_leds_class(void)
{
    if (ensea_leds_class)
        class_destroy(ensea_leds_class);
}


// Specify which device tree devices this driver supports
static struct of_device_id ensea_leds_dt_ids[] = {
    {
        .compatible = "dev,ensea"
    },
    { /* end of table */ }
};

// Inform the kernel about the devices this driver supports
MODULE_DEVICE_TABLE(of, ensea_leds_dt_ids);

// Data structure that links the probe and remove functions with our driver
static struct platform_driver leds_platform = {
    .probe = leds_probe,
    .remove = leds_remove,
    .driver = {
        .name = "Ensea LEDs Driver",
        .owner = THIS_MODULE,
        .of_match_table = ensea_leds_dt_ids
    }
};

// The file operations that can be performed on the ensea_leds character file
static const struct file_operations ensea_leds_fops = {
    .owner = THIS_MODULE,
    .read = leds_read,
    .write = leds_write
};

static const struct file_operations speed_fops = {
    .owner = THIS_MODULE,
    .read = proc_read_speed,
    .write = proc_write_speed,
};

static const struct file_operations direction_fops = {
    .owner = THIS_MODULE,
    .read = proc_read_dir,
    .write = proc_write_dir,
};

// Called when the driver is installed
static int leds_init(void)
{
     int ret_val = 0;
    pr_info("Initializing the Ensea LEDs module\n");

    // Register our driver with the "Platform Driver" bus
    ret_val = platform_driver_register(&leds_platform);
    if(ret_val != 0) {
        pr_err("platform_driver_register returned %d\n", ret_val);
        return ret_val;
    }

    // Create a class for the device
    ensea_leds_class = class_create(THIS_MODULE, "ensea_leds");
    if (IS_ERR(ensea_leds_class)) {
        pr_err("Failed to create class\n");
        ret_val = PTR_ERR(ensea_leds_class);
        goto unregister_driver;
    }

    // Allocate and initialize the cdev structure
    cdev_init(&ensea_leds_cdev, &ensea_leds_fops);

    // Add the cdev to the system
    ret_val = cdev_add(&ensea_leds_cdev, MKDEV(MAJOR_NUMBER, 0), 1);
    if (ret_val < 0) {
        pr_err("Failed to add cdev to system\n");
        goto destroy_class;
    }

    // Create a device node in /dev
    device_create(ensea_leds_class, NULL, MKDEV(MAJOR_NUMBER, 0), NULL, "ensea_leds");

    pr_info("Ensea LEDs module successfully initialized!\n");

    // Créer le répertoire /proc/ensea
    proc_mkdir(PROC_DIR, NULL);

    // Créer le fichier /proc/ensea/speed
    proc_create(PROC_FILE_SPEED, 0666, NULL, &speed_fops);

    // Créer le fichier /proc/ensea/dir
    proc_create(PROC_FILE_DIRECTION, 0666, NULL, &direction_fops);

    return 0;

destroy_class:
    class_destroy(ensea_leds_class);
unregister_driver:
    platform_driver_unregister(&leds_platform);

    return ret_val;
}

// Called whenever the kernel finds a new device that our driver can handle
// (In our case, this should only get called for the one instantiation of the Ensea LEDs module)
static int leds_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;
    struct ensea_leds_dev *dev;
    struct resource *r = 0;

    pr_info("leds_probe enter\n");

    // Get the memory resources for this LED device
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(r == NULL) {
        pr_err("IORESOURCE_MEM (register space) does not exist\n");
        goto bad_exit_return;
    }

    // Create structure to hold device-specific information (like the registers)
    dev = devm_kzalloc(&pdev->dev, sizeof(struct ensea_leds_dev), GFP_KERNEL);

    // Both request and ioremap a memory region
    // This makes sure nobody else can grab this memory region
    // as well as moving it into our address space so we can actually use it
    dev->regs = devm_ioremap_resource(&pdev->dev, r);
    if(IS_ERR(dev->regs))
        goto bad_ioremap;

    // Initialize the misc device (this is used to create a character file in userspace)
    dev->miscdev.minor = MISC_DYNAMIC_MINOR;    // Dynamically choose a minor number
    dev->miscdev.name = "ensea_leds";
    dev->miscdev.fops = &ensea_leds_fops;

    ret_val = misc_register(&dev->miscdev);
    if(ret_val != 0) {
        pr_info("Couldn't register misc device :(");
        goto bad_exit_return;
    }

    // Initialize the timer for chenillard
    setup_timer(&dev->chenillard_timer, chenillard_timer_handler, (unsigned long)dev);

    // Set initial LED value
    dev->leds_value = 0xFF;
    iowrite32(dev->leds_value, dev->regs);

    // Start the chenillard timer with an initial delay based on speed
    mod_timer(&dev->chenillard_timer, jiffies + msecs_to_jiffies(dev->speed));

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void *)dev);

    pr_info("leds_probe exit\n");

    return 0;

bad_ioremap:
    ret_val = PTR_ERR(dev->regs);
bad_exit_return:
    pr_info("leds_probe bad exit :(\n");
    return ret_val;
}


static void chenillard_timer_handler(unsigned long data)
{
    struct ensea_leds_dev *dev = (struct ensea_leds_dev *)data;

    // Logique du chenillard
    if (dev->direction == 0) {
        // Sens horaire
        dev->leds_value = (dev->leds_value << 1) | (dev->leds_value >> 7);
    } else {
        // Sens trigo
        dev->leds_value = (dev->leds_value >> 1) | (dev->leds_value << 7);
    }

    // Appliquer le motif
    dev->leds_value &= dev->pattern;

    // Mettre à jour les LEDs
    iowrite32(dev->leds_value, dev->regs);

    // Programmer le prochain appel du timer avec la vitesse actuelle
    mod_timer(&dev->chenillard_timer, jiffies + msecs_to_jiffies(dev->speed));
}



// This function gets called whenever a read operation occurs on one of the character files
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    int success = 0;
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);

    // Copier le motif depuis dev->pattern vers l'espace utilisateur
    success = copy_to_user(buffer, &dev->pattern, sizeof(dev->pattern));

    // Vérifier si la copie vers l'espace utilisateur a réussi
    if (success != 0) {
        pr_err("Failed to return pattern to userspace\n");
        return -EFAULT;
    }

    return sizeof(dev->pattern);
}


// This function gets called whenever a write operation occurs on one of the character files
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    int success = 0;
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);
    size_t pattern_size = sizeof(dev->pattern);

    if (len > pattern_size) {
        pr_err("Pattern is too large\n");
        return -EINVAL;
    }

    // Copier le motif depuis l'espace utilisateur vers dev->pattern
    success = copy_from_user(&dev->pattern, buffer, len);

    // Vérifier si la copie depuis l'espace utilisateur a réussi
    if (success != 0) {
        pr_err("Failed to read pattern from userspace\n");
        return -EFAULT;
    }

    // Mettre à jour les LEDs avec le nouveau motif
    iowrite32(dev->leds_value & dev->pattern, dev->regs);

    return len;
}



// Définition des fonctions de lecture et d'écriture
static int proc_read_speed(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
    struct ensea_leds_dev *dev = (struct ensea_leds_dev *)data;
    int len = 0;

    len += snprintf(page + len, count - len, "%d\n", dev->speed);
    *eof = 1;  // Indique la fin de fichier

    return len;
}

static int proc_write_speed(struct file *file, const char *buffer, unsigned long count, void *data)
{
    struct ensea_leds_dev *dev = (struct ensea_leds_dev *)data;
    char speed_str[10];
    int len;

    if (count > sizeof(speed_str) - 1)
        return -EINVAL;

    if (copy_from_user(speed_str, buffer, count))
        return -EFAULT;

    speed_str[count] = '\0';
    if (kstrtoint(speed_str, 10, &dev->speed))
        return -EINVAL;

    return count;
}

static int proc_read_dir(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
    struct ensea_leds_dev *dev = (struct ensea_leds_dev *)data;
    int len = 0;

    len += snprintf(page + len, count - len, "%d\n", dev->direction);
    *eof = 1;  // Indique la fin de fichier

    return len;
}

static int proc_write_dir(struct file *file, const char *buffer, unsigned long count, void *data)
{
    struct ensea_leds_dev *dev = (struct ensea_leds_dev *)data;
    char dir_str[2];
    int len;

    if (count > sizeof(dir_str) - 1)
        return -EINVAL;

    if (copy_from_user(dir_str, buffer, count))
        return -EFAULT;

    dir_str[count] = '\0';
    if (kstrtoint(dir_str, 10, &dev->direction))
        return -EINVAL;

    return count;
}




// Gets called whenever a device this driver handles is removed.
// This will also get called for each device being handled when
// our driver gets removed from the system (using the rmmod command).
static int leds_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    struct ensea_leds_dev *dev = (struct ensea_leds_dev*)platform_get_drvdata(pdev);

    pr_info("leds_remove enter\n");

    // Turn the LEDs off
    iowrite32(0x00, dev->regs);

    // Unregister the character file (remove it from /dev)
    misc_deregister(&dev->miscdev);

    pr_info("leds_remove exit\n");

    del_timer_sync(&dev->chenillard_timer);

    return 0;
}

// Called when the driver is removed
static void leds_exit(void)
{
    pr_info("Ensea LEDs module exit\n");

    // Désactiver et libérer le timer pour éviter les fuites de mémoire
    struct ensea_leds_dev *dev = platform_get_drvdata(&leds_platform);
    del_timer_sync(&dev->chenillard_timer);

    // Supprimer les fichiers proc
    remove_proc_entry("speed", NULL);
    remove_proc_entry("dir", NULL);

    // Éteindre les LEDs
    iowrite32(0x00, dev->regs);

    // Désenregistrer le périphérique misc
    misc_deregister(&dev->miscdev);

    pr_info("Ensea LEDs module successfully unregistered\n");
}


// Tell the kernel which functions are the initialization and exit functions
module_init(leds_init);
module_exit(leds_exit);

// Define information about this kernel module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Devon Andrade <devon.andrade@oit.edu>");
MODULE_DESCRIPTION("Exposes a character device to user space that lets users turn LEDs on and off");
MODULE_VERSION("1.0");