KERNEL_DIR=../ebf-buster-linux/build_image/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m :=get_dts_info.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

.PHONE:clean 

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean	