#ifndef GPIO_H
#define GPIO_H


// Returns 0 if succesful or -1 in case of error
int pinMode(const char* pin, const char *mode);

// Disables the pin
// Returns 0 if succesful or -1 in case of error
int disablePin(const char *pin);

// Change the digital output of the pin (0 or 1)
// Returns 0 if succesful or -1 in case of error
int digitalWrite(const char* pin, const char* value);

// Read the digital input value of the pin
// Returns the read value (0 or 1) or -1 in case of error
int digitalRead(const char* pin, char* value);

// Change the digital value of a GPIO at the given frequency (in Hz), 
// for the specified duration (in milliseconds)
// Returns 0 if succesful or -1 in case of error
int blink(const char *pin, unsigned int freq, unsigned int duration);


// Waits for a pin to change its value and then meassures for how long
// it's that value hold
// Returns the time in microseconds or -1 if it failed or timeout
int pulseIn(const char *pin, const char* state, unsigned int timeout);


// Return the distance captured by the ultrasonic sensor,
// -1 if error, or 0 if timeout
int getDistance(const char *trigger_pin, const char *echo_pin);


// Takes a picture with fswebcam returns -1 if error
int takePicture(const char *filename);

#endif // GPIO_H
