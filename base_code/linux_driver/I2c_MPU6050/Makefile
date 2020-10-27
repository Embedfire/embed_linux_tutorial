KERNEL_DIR=../ebf-buster-linux/build_image/build

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
export  ARCH  CROSS_COMPILE

obj-m := i2c_mpu6050.o

out =  6050_test_app

all:
	$(MAKE) EXTRA_CFLAGS=-fno-pic -C $(KERNEL_DIR) M=$(CURDIR) modules
	$(CROSS_COMPILE)gcc -o $(out) test_app.c
	
.PHONY:clean
clean:
	$(MAKE)  -C $(KERNEL_DIR) M=$(CURDIR) clean
		rm $(out)
