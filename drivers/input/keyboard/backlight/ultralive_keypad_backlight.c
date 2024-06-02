#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/slab.h>

#define OUTPUT_GPIO_PIN 38 // GPIO pin for the output signal

static struct input_dev *button_dev;
static unsigned int output_gpio_num = OUTPUT_GPIO_PIN;
static struct timer_list gpio_timer;

static void gpio_timer_callback(unsigned long data)
{
    gpio_set_value(output_gpio_num, 0); // Set the output GPIO pin low
}



static void buttons_input_event(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
    if (type == EV_KEY && value == 1) { // Check for key press event
        gpio_set_value(output_gpio_num, 1); // Set the output GPIO pin high
        mod_timer(&gpio_timer, jiffies + msecs_to_jiffies(10000)); // Reset the timer for 10 seconds
    }
}


static int buttons_input_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id)
{
    struct input_handle *handle;
    int error;

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle)
        return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "buttons_backlight";

    error = input_register_handle(handle);
    if (error)
        goto err_free_handle;

    error = input_open_device(handle);
    if (error)
        goto err_unregister_handle;

    button_dev = dev;
    return 0;

err_unregister_handle:
    input_unregister_handle(handle);
err_free_handle:
    kfree(handle);
    return error;
}

static void buttons_input_disconnect(struct input_handle *handle)
{
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
}

static const struct input_device_id buttons_ids[] = {
    {
        .flags = INPUT_DEVICE_ID_MATCH_EVBIT | INPUT_DEVICE_ID_MATCH_KEYBIT,
        .evbit = { BIT_MASK(EV_KEY) },
        .keybit = {
            [BIT_WORD(KEY_3)] = BIT_MASK(KEY_3),
            [BIT_WORD(KEY_4)] = BIT_MASK(KEY_4),
            [BIT_WORD(KEY_Q)] = BIT_MASK(KEY_Q),
            [BIT_WORD(KEY_W)] = BIT_MASK(KEY_W),
            [BIT_WORD(KEY_E)] = BIT_MASK(KEY_E),
            [BIT_WORD(KEY_R)] = BIT_MASK(KEY_R),
            [BIT_WORD(KEY_U)] = BIT_MASK(KEY_U),
            [BIT_WORD(KEY_I)] = BIT_MASK(KEY_I),
            [BIT_WORD(KEY_A)] = BIT_MASK(KEY_A),
            [BIT_WORD(KEY_S)] = BIT_MASK(KEY_S),
            [BIT_WORD(KEY_D)] = BIT_MASK(KEY_D),
            [BIT_WORD(KEY_F)] = BIT_MASK(KEY_F),
            [BIT_WORD(KEY_J)] = BIT_MASK(KEY_J),
            [BIT_WORD(KEY_K)] = BIT_MASK(KEY_K),
            [BIT_WORD(KEY_L)] = BIT_MASK(KEY_L),
            [BIT_WORD(KEY_C)] = BIT_MASK(KEY_C),
            [BIT_WORD(KEY_V)] = BIT_MASK(KEY_V),
            [BIT_WORD(KEY_N)] = BIT_MASK(KEY_N),
            [BIT_WORD(KEY_M)] = BIT_MASK(KEY_M),
            [BIT_WORD(KEY_COMMA)] = BIT_MASK(KEY_COMMA),
        },
    },
    { },
};

static struct input_handler buttons_input_handler = {
    .event          = buttons_input_event,
    .connect        = buttons_input_connect,
    .disconnect     = buttons_input_disconnect,
    .name           = "buttons_backlight_handler",
    .id_table       = buttons_ids,
};

static int __init gpio_controller_init(void)
{
    int ret;

    // Request the output GPIO pin
    ret = gpio_request(output_gpio_num, "output_gpio");
    if (ret) {
        pr_err("Failed to request output GPIO\n");
        return ret;
    }

    // Set the output GPIO pin direction to output
    ret = gpio_direction_output(output_gpio_num, 0);
    if (ret) {
        pr_err("Failed to set output GPIO direction\n");
        gpio_free(output_gpio_num);
        return ret;
    }

    // Initialize the GPIO timer
    setup_timer(&gpio_timer, gpio_timer_callback, 0);

    // Set the output GPIO pin high initially for 10 seconds
    gpio_set_value(output_gpio_num, 1);
    mod_timer(&gpio_timer, jiffies + msecs_to_jiffies(10000));

    // Register the input handler
    ret = input_register_handler(&buttons_input_handler);
    if (ret) {
        pr_err("Failed to register input handler\n");
        gpio_free(output_gpio_num);
        return ret;
    }

    pr_info("GPIO controller module loaded\n");
    return 0;
}

static void __exit gpio_controller_exit(void)
{
    // Unregister the input handler
    input_unregister_handler(&buttons_input_handler);

    // Free the GPIO pin
    gpio_set_value(output_gpio_num, 0);
    gpio_free(output_gpio_num);

    // Delete the timer
    del_timer(&gpio_timer);

    pr_info("GPIO controller module unloaded\n");
}

module_init(gpio_controller_init);
module_exit(gpio_controller_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Christopher Bradski");
MODULE_DESCRIPTION("GPIO Backlight Controller Module");
