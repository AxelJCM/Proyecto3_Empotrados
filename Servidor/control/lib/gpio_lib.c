#include "gpio_lib.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>


int writeToFile(const char *absoluteFileName, const char *contents) {
	int fd = open(absoluteFileName, O_WRONLY | O_TRUNC);

	if (-1 == fd) {
		fprintf(stderr, "Couldn't open %s for writing!\n", absoluteFileName);
		return -1;
	}

	int contentsLength = strlen(contents);

	if (write(fd, contents, contentsLength) != contentsLength) {
		fprintf(stderr, "Failed to write entire value %s to %s!\n", contents, absoluteFileName);
		close(fd);
		return -1;
	}

	close(fd);
    return 0;
}

int pinMode(const char* pin, const char *mode){
    if (writeToFile("/sys/class/gpio/export", pin) != 0) {
        return -1;
    }

	char buf[256];
	struct stat statBuf;
	int pinExported = -1;

	sprintf(buf,"/sys/class/gpio/gpio%s/direction", pin);
	// May have to briefly wait for OS to make symlink! 
	while (pinExported != 0) {	
		sleep(1);
		pinExported = stat(buf, &statBuf);
	}

    if(strcmp(mode,"out")==0 || strcmp(mode,"in")==0){
        if (writeToFile(buf, mode) != 0){
            return -1;
        }
    }
    else {
        fprintf(stderr, "Invalid mode '%s'. Only use 'in' or 'out'.\n", mode);
        return -1;
    }

    return 0;
}


int disablePin(const char *pin)
{
    if (writeToFile("/sys/class/gpio/unexport", pin) != 0) {
        return -1;
    }
    return 0;
}


int digitalWrite(const char* pin, const char* value){
    if(strcmp(value,"0")==0 || strcmp(value,"1")==0){
        char buffer[256];
        sprintf(buffer,"/sys/class/gpio/gpio%s/value", pin);
        if (writeToFile(buffer,value) != 0){
            return -1;
        }
    }else{
        fprintf(stderr, "Invalid value '%s'. Valid values are '1' or '0'\n", value);
        return -1;
    }
    return 0;
}


int digitalRead(const char* pin, char* value){
    char buffer[256];
    int fd;
    ssize_t bytesRead;

    // Construct the path to the value file
    sprintf(buffer, "/sys/class/gpio/gpio%s/value", pin);

    // Open the file for reading
    fd = open(buffer, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open pin value file.");
        return -1;
    }

    // Read the value from the file
    bytesRead = read(fd, value, sizeof(value) - 1);
    if (bytesRead == -1) {
        perror("Failed to read pin value.");
        close(fd);
        return -1;
    }

    // Null-terminate the string
    value[bytesRead-1] = '\0';

    close(fd);
    return 0;
}


int blink(const char *pin, unsigned int freq, unsigned int duration)
{
    if (freq == 0){  // Avoids division by 0.
        perror("A frequency of 0 is not valid.");
        return -1;
    }

    int period = 1000000 / freq;  // microseconds
    int cycles = freq * duration/1000;
    
    for (int i = 0; i < cycles; i++) {
        if (digitalWrite(pin, "1") != 0){
            return -1;
        }
        usleep(period/2);
        if (digitalWrite(pin, "0") != 0){
            return -1;
        }
        usleep(period/2);
    }

    return 0;
}


// Get the current time in microseconds
unsigned int getMicros() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000UL) + tv.tv_usec;
}


int pulseIn(const char *pin, const char* state, unsigned int timeout)
{
    unsigned int startTime = getMicros();
    unsigned int endTime;

    char value[4];
    if (digitalRead(pin, value) != 0) {
        return -1;
    }

    // Wait for the pin to go to the desired state
    while (strcmp(value, state) != 0) {
        if (digitalRead(pin, value) != 0) {
            return -1;
        }
        if ((getMicros() - startTime) > timeout) {
            perror("Timeout.");
            return 0;  // Timeout occurred
        }
    }

    // Pin is now in the desired state, start measuring
    unsigned int pulseStart = getMicros();
    if (digitalRead(pin, value) != 0) {
        return -1;
    }
    
    // Wait for the pin to change back to the opposite state
    while (strcmp(value, state) == 0) {
        if (digitalRead(pin, value) != 0) {
            return -1;
        }
        if ((getMicros() - pulseStart) > timeout) {
            perror("Timeout.");
            return 0;  // Timeout occurred
        }
    }


    // Calculate pulse duration in microseconds
    endTime = getMicros();
    return endTime - pulseStart;
}


int getDistance(const char *trigger_pin, const char *echo_pin)
{
    int t; // echo time
    int d; // distance(cm)

    digitalWrite(trigger_pin, "1");
    usleep(10);       // Send a pulse of 10us
    digitalWrite(trigger_pin, "0");

    t = pulseIn(echo_pin, "1", 1000000);
    d = t/59;

    return d;
}


int takePicture(const char *filename) {
    // Construct the command string
    char command[256];
    snprintf(command, sizeof(command), "fswebcam %s", filename);
    
    // Execute the command
    int result = system(command);
    
    // Check for errors
    if (result == -1) {
        perror("system");
        return -1;
    } else if (result != 0) {
        fprintf(stderr, "Command failed with status %d\n", result);
        return -1;
    }
    return 0;
}