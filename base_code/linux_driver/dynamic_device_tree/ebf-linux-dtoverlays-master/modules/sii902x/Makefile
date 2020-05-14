

_uname_r := $(shell uname -r)
_BUILD   := /lib/modules/$(_uname_r)/build
_PWD     := $(shell pwd)
_DEPMOD  ?= depmod -a

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language
ifneq ($(KERNELRELEASE),)


obj-m += mxsfb_sii902x.o

else
_class  := drivers/input

DEST  := /lib/modules/$(_uname_r)/kernel

all:
	make -C ${_BUILD} M=$(_PWD) modules

clean:
	make -C ${_BUILD} M=$(_PWD) clean

install:
	cp mxsfb_sii902x.ko ${DEST}/${_class}/
	$(_DEPMOD)

uninstall:
	rm -f ${DEST}/${_class}/mxsfb_sii902x.ko 
	$(_DEPMOD)

.PHONY: all clean install uninstall
endif
