
_uname_r := $(shell uname -r)
_BUILD   := /lib/modules/$(_uname_r)/build
_PWD     := $(shell pwd)
_DEPMOD  ?= depmod -a

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language
ifneq ($(KERNELRELEASE),)

#snd-soc-imx-wm8960-objs := imx-wm8960.o
obj-m += ov5640_v2.o

else
_class  := drivers/i2c

DEST  := /lib/modules/$(_uname_r)/kernel

all:
	make -C ${_BUILD} M=$(_PWD) modules

clean:
	make -C ${_BUILD} M=$(_PWD) clean

install:
	cp ov5640_v2.ko ${DEST}/${_class}/
	$(_DEPMOD)
	echo ov5640_v2.ko >> /etc/modules
uninstall:
	rm -f ${DEST}/${_class}/ov5640_v2.ko 
	$(_DEPMOD)

.PHONY: all clean install uninstall
endif
