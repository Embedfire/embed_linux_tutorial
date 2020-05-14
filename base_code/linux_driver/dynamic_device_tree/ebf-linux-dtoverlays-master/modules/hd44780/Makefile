#
# Peter Yang <turmary@126.com>
# Copyright (c) 2019 Seeed Studio
#
# MIT License
#

_tgt     := hd44780

_uname_r := $(shell uname -r)
_BUILD   := /lib/modules/$(_uname_r)/build
_PWD     := $(shell pwd)
_DEPMOD  ?= depmod -a

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language
ifneq ($(KERNELRELEASE),)
hd44780-objs := hd44780-i2c.o hd44780-dev.o
obj-m += ${_tgt}.o

else
_class  := drivers/misc

DEST  := /lib/modules/$(_uname_r)/kernel

all:
	make -C ${_BUILD} M=$(_PWD) modules

clean:
	make -C ${_BUILD} M=$(_PWD) clean

install:
	cp ${_tgt}.ko ${DEST}/${_class}/
	$(_DEPMOD)

uninstall:
	rm -f ${DEST}/${_class}/${_tgt}.ko
	$(_DEPMOD)

.PHONY: all clean install uninstall
endif
