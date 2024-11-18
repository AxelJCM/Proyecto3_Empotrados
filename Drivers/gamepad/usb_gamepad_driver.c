
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/hid.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h> 

#define USB_GAMEPAD_VENDOR_ID 0x0583
#define USB_GAMEPAD_PRODUCT_ID 0xa009
#define GAMEPAD_CHAR_DEV "gamepad_dev"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Simon_Rachel_Pablo_Axel");
MODULE_DESCRIPTION("USB gamepad driver");


static int __init gamepad_init(void);
static void __exit gamepad_exit(void);
static int gamepad_probe(struct hid_device *hdev, const struct hid_device_id *id);
static void gamepad_disconnect(struct hid_device *hdev);
static int gamepad_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size);
static ssize_t gamepad_read(struct file *file, char __user *buffer, size_t len, loff_t *offset);


// USB device ID table
static const struct hid_device_id gamepad_table[] = {
    { HID_USB_DEVICE(USB_GAMEPAD_VENDOR_ID, USB_GAMEPAD_PRODUCT_ID) },
    {}
};
MODULE_DEVICE_TABLE(hid, gamepad_table);


// Define the HID driver structure
static struct hid_driver gamepad_driver = 
{
    .name = "usb_gamepad_driver",
    .id_table = gamepad_table,
    .probe = gamepad_probe,
    .remove = gamepad_disconnect,
    .raw_event = gamepad_raw_event,
};

static struct file_operations fops = {
    .read = gamepad_read,
};

static struct class *gamepad_class = NULL;
static struct device *gamepad_device = NULL;



/* Declaration of the init and exit functions */
module_init(gamepad_init);
module_exit(gamepad_exit);


/* Global variables */
static int major_number;
static unsigned char button_pressed = 0; // Stores the pressed button number
static unsigned char button_prev_state = 0;


static char *gamepad_devnode(const struct device *dev, umode_t *mode)
{
    if (mode)
        *mode = 0666; // Set permissions to rw-rw-rw-
    return NULL;
}


static int __init gamepad_init(void) 
{
    int ret;

    // Character device registration
    major_number = register_chrdev(0, GAMEPAD_CHAR_DEV, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register char device: %d\n", major_number);
        return major_number;
    }

    // Device class creation
    gamepad_class = class_create("gamepad_class");
    if (IS_ERR(gamepad_class)) {
        printk(KERN_ERR "Failed to create device class\n");
        unregister_chrdev(major_number, GAMEPAD_CHAR_DEV);
        return PTR_ERR(gamepad_class);
    }
    gamepad_class->devnode = gamepad_devnode;

    // Device creation
    gamepad_device = device_create(gamepad_class, NULL, MKDEV(major_number, 0), NULL, GAMEPAD_CHAR_DEV);
    if (IS_ERR(gamepad_device)) {
        printk(KERN_ERR "Failed to create device\n");
        class_destroy(gamepad_class);
        unregister_chrdev(major_number, GAMEPAD_CHAR_DEV);
        return PTR_ERR(gamepad_device);
    }

    // HID driver registration
    ret = hid_register_driver(&gamepad_driver);
    if (ret) {
        printk(KERN_ERR "Failed to register HID driver: %d\n", ret);
        device_destroy(gamepad_class, MKDEV(major_number, 0));
        class_destroy(gamepad_class);
        unregister_chrdev(major_number, GAMEPAD_CHAR_DEV);
        return ret;
    }

    printk(KERN_INFO "Gamepad driver initialized.\n");
    printk(KERN_INFO "Character device: /dev/gamepad_dev. Mayor number: %d\n", major_number);

    return 0;
}


static void __exit gamepad_exit(void) 
{
    hid_unregister_driver(&gamepad_driver);
    device_destroy(gamepad_class, MKDEV(major_number, 0));
    class_destroy(gamepad_class);
    unregister_chrdev(major_number, GAMEPAD_CHAR_DEV);

    printk(KERN_INFO "Gamepad driver exited.");
}


static int gamepad_probe(struct hid_device *hdev, const struct hid_device_id *id) 
{
    int ret;

    printk(KERN_INFO "USB gamepad (%04X:%04X) connected\n", id->vendor, id->product);

    // Initialize the HID device
    ret = hid_parse(hdev);
    if (ret) {
        printk(KERN_ERR "Failed to parse HID device\n");
        return ret;
    }

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        printk(KERN_ERR "Failed to start HID hardware\n");
        return ret;
    }

    ret = hid_hw_open(hdev);
    if (ret) {
        printk(KERN_ERR "Failed to open HID device\n");
        hid_hw_stop(hdev);
        return ret;
    }

    return 0;
}




static void gamepad_disconnect(struct hid_device *hdev) 
{
    printk(KERN_INFO "USB gamepad disconnected\n");
    hid_hw_close(hdev);
    hid_hw_stop(hdev);
}



// Event handler for HID reports (captures button states)
static int gamepad_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size) 
{
    if (data[5] != 0){
        button_prev_state = data[5];
        return 0;
    }
    
    if (data[5] == 0 && button_prev_state != 0){
        button_pressed = button_prev_state;
        button_prev_state = 0;
        printk(KERN_INFO "Button %d pressed\n", button_pressed);
    }

    return 0;
}



static ssize_t gamepad_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    int ret;

    // Copy button status to user space
    ret = copy_to_user(buffer, &button_pressed, sizeof(button_pressed));
    if (ret != 0) 
        return -EFAULT;

    // Clear button status after reading
    button_pressed = 0;

    return sizeof(button_pressed);
}
