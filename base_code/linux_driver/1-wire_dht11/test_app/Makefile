KERNEL_DIR = /home/xiaojian/workspace/imx6ull/linux/linux4.19/company/ebf-buster-linux/ebf-buster-linux/build_image/build

obj-m := dht11.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	
.PHONY:clean
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean