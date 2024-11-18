
# Module name
obj-m += usb_gamepad_driver.o

# Path to the kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# Path to the current directory
PWD := $(shell pwd)

# Default target to build the module
all:
	make -C $(KDIR) M=$(PWD) modules
	mkdir -p output
	mv usb_gamepad_driver.ko output/

# Clean target to remove generated files
clean:
	rm -rf *.o *.mod* *.order *.symvers .*.cmd