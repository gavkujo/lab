obj-m += hello_lkm.o
obj-m += gpio_lkm.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# For Raspberry Pi, you might need to specify the kernel source path
# KDIR := /path/to/raspberrypi/linux

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install-hello:
	sudo insmod hello_lkm.ko

install-gpio:
	sudo insmod gpio_lkm.ko

uninstall-hello:
	sudo rmmod hello_lkm

uninstall-gpio:
	sudo rmmod gpio_lkm

help:
	@echo "Available targets:"
	@echo "  all          - Build all modules"
	@echo "  clean        - Clean build files"
	@echo "  install-hello - Load hello_lkm module"
	@echo "  install-gpio  - Load gpio_lkm module"
	@echo "  uninstall-*  - Unload respective module"