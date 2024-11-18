#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

int main() {
    int fd;
    unsigned char button_pressed;

    // Open the character device
    fd = open("/dev/gamepad_dev", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    while (1) {
        // Read button state from the device
        if (read(fd, &button_pressed, sizeof(button_pressed)) > 0) {
            if (button_pressed != 0) 
                printf("Button %d pressed\n", button_pressed);
            if (button_pressed == 0x04)
                break;
        }
        usleep(100000);  // Sleep to reduce CPU usage
    }

    close(fd);
    return 0;
}
