KERNEL_DIR = /home/book/embedfire/imx6ull/linuxcore/ebf-buster-linux-master

obj-m := dht11.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	
.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean