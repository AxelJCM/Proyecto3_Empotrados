from ctypes import *
import time

so_file = "/usr/lib/libgpio.so.0"
#so_file = "/lib/libgpio_d.so"
func = CDLL(so_file)

# Returns 0 if succesful or -1 in case of error
def pinMode(pin:bytes, mode:bytes):
    return func.pinMode(pin, mode)

# Disables the pin
# Returns 0 if succesful or -1 in case of error
def disablePin(pin:bytes):
    return func.disablePin(pin)

# Change the digital output of the pin (0 or 1)
# Returns 0 if succesful or -1 in case of error
def digitalWrite(pin:bytes, value:bytes):
    return func.digitalWrite(pin, value)

#  Read the digital input value of the pin
#  Returns the read value (0 or 1) or -1 in case of error
def digitalRead(pin:bytes, value):
    return func.digitalRead(pin, value)

# Change the digital value of a GPIO at the given frequency (in Hz), 
# for the specified duration (in milliseconds)
# Returns 0 if succesful or -1 in case of error
def blink(pin:bytes, freq:int, duration:int):
    return func.blink(pin, freq, duration)


# Waits for a pin to change its value and then meassures for how long
# it's that value hold
# Returns the time in microseconds or -1 if it failed or timeout
def pulseIn(pin:bytes, state:bytes, timeout:int):
    return func.pulseIn(pin, state, timeout)


# Return the distance captured by the ultrasonic sensor,
# -1 if error, or 0 if timeout
def getDistance(trigger_pin:bytes, echo_pin:bytes):
    return func.getDistance(trigger_pin, echo_pin)


def takePicture(name:bytes):
    return func.takePicture(name)