

NATIVE ?= true


ifeq ($(NATIVE), false)
	KERNEL_DIR = /home/embedfire/linux4.19
else
	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
endif

obj-m := led_pdev.o led_pdrv.o

all:modules
modules clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(shell pwd) $@



