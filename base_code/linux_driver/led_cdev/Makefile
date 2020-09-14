KERNEL_DIR=../ebf-buster-linux/build_image/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m := led_cdev.o
out =  led_cdev_test

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
	$(CROSS_COMPILE)gcc -o $(out) led_test.c

.PHONE:clean copy

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean	
	rm $(out)



