
_uname_r := $(shell uname -r)
_BUILD   := /lib/modules/$(_uname_r)/build
_PWD     := $(shell pwd)
_DEPMOD  ?= depmod -a

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language
ifneq ($(KERNELRELEASE),)

touch-gt9xx-objs := gt9xx.o gt9xx_update.o goodix_tool.o
obj-m += touch-gt9xx.o

else
_class  := drivers/input

DEST  := /lib/modules/$(_uname_r)/kernel

all:
	make -C ${_BUILD} M=$(_PWD) modules

clean:
	make -C ${_BUILD} M=$(_PWD) clean

install:
	cp touch-gt9xx.ko ${DEST}/${_class}/
	$(_DEPMOD)

uninstall:
	rm -f ${DEST}/${_class}/touch-gt9xx.ko 
	$(_DEPMOD)

.PHONY: all clean install uninstall
endif
