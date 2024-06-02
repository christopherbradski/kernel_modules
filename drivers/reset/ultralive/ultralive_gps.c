#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define GPS_RESET_GPIO 64 // Reset pin on the ultralive board

static int __init gps_reset_init(void)
{
    int ret;

    // Request the GPIO pin
    ret = gpio_request(GPS_RESET_GPIO, "gps_reset");
    if (ret < 0) {
        printk(KERN_ERR "Failed to request GPIO %d\n", GPS_RESET_GPIO);
        return ret;
    }

    // Set GPIO direction to output
    ret = gpio_direction_output(GPS_RESET_GPIO, 1); // Initially high
    if (ret < 0) {
        printk(KERN_ERR "Failed to set GPIO direction\n");
        gpio_free(GPS_RESET_GPIO);
        return ret;
    }

    // Reset GPS device
    gpio_set_value(GPS_RESET_GPIO, 0); // Pull low to reset
    msleep(500); // Wait for 500 milliseconds
    gpio_set_value(GPS_RESET_GPIO, 1); // Release reset (pull high)

    printk(KERN_INFO "GPS reset completed\n");

    return 0;
}

static void __exit gps_reset_exit(void)
{
    // Free GPIO pin
    gpio_free(GPS_RESET_GPIO);

    printk(KERN_INFO "GPS reset module unloaded\n");
}

module_init(gps_reset_init);
module_exit(gps_reset_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christopher Bradski");
MODULE_DESCRIPTION("Kernel module to reset the ublox GPS device on port 64 of the ultralive board");
