KERNEL_DIR=../ebf-buster-linux/build_image/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m := led_pdev.o led_pdrv.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
modules clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean	
