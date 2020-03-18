
obj-m := led_cdev.o


NATIVE ?= true


ifeq ($(NATIVE), false)
	KERNEL_DIR = /home/embedfire/linux4.19
else
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

all:modules
modules clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(shell pwd) $@
