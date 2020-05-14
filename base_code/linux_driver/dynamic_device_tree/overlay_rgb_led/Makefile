KERNEL_DIR = /home/new_kernel2/ebf-buster-linux

obj-m := rgb_led.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	
.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean
