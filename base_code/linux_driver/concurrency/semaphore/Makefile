KERNEL_DIR = /home/fire2/ebf-buster-linux

obj-m := semaphore.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	
.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

	echo 1 >/dev/led_chrdev0
