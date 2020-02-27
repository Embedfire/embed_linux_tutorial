

KERNEL_DIR = /home/wind/ebf_6ull_linux

obj-m := my_pdev.o my_pdrv.o

all:modules
modules clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(shell pwd) $@



