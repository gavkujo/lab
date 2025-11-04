#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <string.h>
#include <inttypes.h>

int main()
{
    int fd0 = open("/dev/gpiochip0", O_RDWR);
    if (fd0 < 0) {
        perror("Failed to open /dev/gpiochip0");
        return 1;
    }

    // Setup Red LED
    struct gpiohandle_request req_led;
    struct gpiohandle_data data_led;
    memset(&req_led, 0, sizeof(req_led));
    memset(&data_led, 0, sizeof(data_led));
    
    req_led.lines = 1;
    req_led.lineoffsets[0] = 27;  // Red LED
    req_led.flags = GPIOHANDLE_REQUEST_OUTPUT;
    data_led.values[0] = 0;  // OFF
    
    if (ioctl(fd0, GPIO_GET_LINEHANDLE_IOCTL, &req_led) < 0) {
        perror("Failed to setup Red LED");
        close(fd0);
        return 1;
    }

    // Setup Button Event
    struct gpioevent_request req_btn;
    struct gpioevent_data event;
    memset(&req_btn, 0, sizeof(req_btn));
    
    req_btn.lineoffset = 11;  // Push Button
    req_btn.handleflags = GPIOHANDLE_REQUEST_INPUT;
    req_btn.eventflags = GPIOEVENT_REQUEST_RISING_EDGE;
    strcpy(req_btn.consumer_label, "Button");
    
    if (ioctl(fd0, GPIO_GET_LINEEVENT_IOCTL, &req_btn) < 0) {
        perror("Failed to setup button event");
        close(req_led.fd);
        close(fd0);
        return 1;
    }

    printf("Ready! Press the button to blink Red LED...\n");

    while (1) {
        // Wait for button press
        if (read(req_btn.fd, &event, sizeof(event)) == sizeof(event)) {
            printf("Button pressed! Blinking Red LED...\n");
            
            // Blink 5 times
            for (int i = 0; i < 5; i++) {
                data_led.values[0] = 1; // ON
                ioctl(req_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_led);
                usleep(500000);
                
                data_led.values[0] = 0; // OFF
                ioctl(req_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data_led);
                usleep(500000);
            }
            printf("Done.\n");
        }
    }

    close(req_btn.fd);
    close(req_led.fd);
    close(fd0);
    return 0;
}
