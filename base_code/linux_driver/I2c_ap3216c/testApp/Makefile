KERNEL_DIR := /home/book/embedfire/imx6ull/linuxcore/ebf-buster-linux-master

obj-m := i2c_ap3216c.o

all:
	$(MAKE) EXTRA_CFLAGS=-fno-pic -C $(KERNEL_DIR) M=$(CURDIR) modules
	
.PHONY:clean
clean:
	$(MAKE)  -C $(KERNEL_DIR) M=$(CURDIR) clean