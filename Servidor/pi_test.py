import ctypes as ct
import time
from control import *

# Pin configuration

pin1 = b"529"   # GPIO17
pin2 = b"539"   # GPIO27
pin3 = b"534"   # GPIO22

pinMode(pin1, b"out")
pinMode(pin2, b"in")
pinMode(pin3, b"out")

# Test
print("Starting test.")

# Set pin1 to 1, then 0 after 2 seconds
digitalWrite(pin1, b"1")
time.sleep(2)
digitalWrite(pin1, b"0")

# Set pin3 to blink for 5 seconds at a 2Hz frequency
print("Blinking...")
blink(pin3, 1, 5000)


# Reads the value from pin2
value_read = ct.create_string_buffer(4) # string buffer
digitalRead(pin2, value_read)
print("Pin", pin2.decode('utf-8'), "value:", value_read.value.decode('utf-8'))


# Disable the pins
disablePin(pin1)
disablePin(pin2)
disablePin(pin3)



# Distance Test

trigger_pin = b"535"    # GPIO23
echo_pin = b"536"       # GPIO24

pinMode(trigger_pin, b"out")
pinMode(echo_pin, b"in")


# Check distance each second for 10 seconds
for i in range(10):
    print("Distance:", getDistance(trigger_pin, echo_pin))
    time.sleep(1)

# Disable the pins
disablePin(trigger_pin)
disablePin(echo_pin)


# Take picture test

takePicture(b"hola.jpg")