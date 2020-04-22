KERNEL_DIR=/home/embedfire/module/linux-imx
obj-m := hello_world.o
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean	
