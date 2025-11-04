#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CE3103");
MODULE_DESCRIPTION("GPIO LKM with Interrupt Handling");
MODULE_VERSION("V1");

static unsigned int ledGreen = 4;    // GPIO4 (pin 7) - Green LED
static unsigned int pushButton = 11; // GPIO11 (pin 23) - Push Button
static unsigned int irqNumber;       // IRQ number for button
static bool ledOn = 0;              // LED state tracker

// GPIO IRQ Handler function
static irq_handler_t rpi_gpio_isr(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
    // Toggle the LED state
    ledOn = !ledOn;
    
    // Set LED accordingly
    gpio_set_value(ledGreen, ledOn);
    
    printk(KERN_INFO "GPIO Interrupt! Button pressed. LED state: %s\n", 
           ledOn ? "ON" : "OFF");
    
    return (irq_handler_t) IRQ_HANDLED;
}

// LKM initialization function
static int __init rpi_gpio_init(void)
{
    int result = 0;
    printk(KERN_INFO "Initializing the GPIO LKM\n");

    // Check if GPIO numbers are valid
    if (!gpio_is_valid(ledGreen)) {
        printk(KERN_ALERT "Invalid LED GPIO: %d\n", ledGreen);
        return -ENODEV;
    }
    
    if (!gpio_is_valid(pushButton)) {
        printk(KERN_ALERT "Invalid Button GPIO: %d\n", pushButton);
        return -ENODEV;
    }

    ledOn = true; // Default for LED is ON

    // Request LED GPIO
    if (gpio_request(ledGreen, "sysfs")) {
        printk(KERN_ALERT "Failed to request LED GPIO %d\n", ledGreen);
        return -ENODEV;
    }

    // Set LED as output and turn on initially
    if (gpio_direction_output(ledGreen, ledOn)) {
        printk(KERN_ALERT "Failed to set LED GPIO direction\n");
        gpio_free(ledGreen);
        return -ENODEV;
    }

    // Request Button GPIO
    if (gpio_request(pushButton, "sysfs")) {
        printk(KERN_ALERT "Failed to request Button GPIO %d\n", pushButton);
        gpio_free(ledGreen);
        return -ENODEV;
    }

    // Set Button as input
    if (gpio_direction_input(pushButton)) {
        printk(KERN_ALERT "Failed to set Button GPIO direction\n");
        gpio_free(ledGreen);
        gpio_free(pushButton);
        return -ENODEV;
    }

    // Set debounce delay
    if (gpio_set_debounce(pushButton, 1000)) {
        printk(KERN_WARNING "Failed to set debounce, continuing anyway\n");
    }

    // Map pushbutton to IRQ number
    irqNumber = gpio_to_irq(pushButton);
    printk(KERN_INFO "Button mapped to IRQ: %d\n", irqNumber);

    // Request interrupt
    result = request_irq(irqNumber,                    // interrupt number
                        (irq_handler_t) rpi_gpio_isr,  // ISR handler function
                        IRQF_TRIGGER_RISING,           // trigger on rising edge
                        "rpi_gpio_handler",            // used in /proc/interrupts
                        NULL);                         // dev_id for shared IRQ

    if (result) {
        printk(KERN_ALERT "Failed to request IRQ %d, error: %d\n", irqNumber, result);
        gpio_free(ledGreen);
        gpio_free(pushButton);
        return result;
    }

    printk(KERN_INFO "GPIO LKM successfully loaded\n");
    printk(KERN_INFO "LED on GPIO%d, Button on GPIO%d\n", ledGreen, pushButton);
    
    return 0;
}

// LKM exit function
static void __exit rpi_gpio_exit(void)
{
    // Turn off LED
    gpio_set_value(ledGreen, 0);
    
    // Free IRQ
    free_irq(irqNumber, NULL);
    
    // Free GPIOs
    gpio_free(ledGreen);
    gpio_free(pushButton);
    
    printk(KERN_INFO "GPIO LKM unloaded. Goodbye!\n");
}

module_init(rpi_gpio_init);
module_exit(rpi_gpio_exit);