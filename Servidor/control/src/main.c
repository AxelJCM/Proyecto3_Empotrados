#include "gpio_lib.h"
#include <stdio.h>
#include <unistd.h>

int main(){
    const char* pin1 = "529";   // GPIO17
    const char* pin2 = "539";    // GPIO27
    const char* pin3 = "534";   // GPIO22

    // Pin configuration
    pinMode(pin1,"out");
    pinMode(pin2,"in");
    pinMode(pin3,"out");

    // Test
    printf("Starting test.\n\n");

    // Set pin1 to 1, then 0 after 2 seconds
    digitalWrite(pin1,"1");
    sleep(2);
    digitalWrite(pin1,"0");
    
    // Set pin3 to blink for 5 seconds at a 2Hz frequency
    printf("Blinking...\n");
    blink(pin3, 1, 5000);

    
    // Reads the value from pin2
    char value[4];
    digitalRead(pin2, value);
    printf("Pin %s value: %s\n", pin2, value);


    disablePin(pin1);
    disablePin(pin2);
    disablePin(pin3);

    return 0;
}