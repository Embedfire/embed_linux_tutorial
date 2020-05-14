#
# Makefile for the Dallas's 1-wire bus.
#

#
#Copyright (C) 2019 Seeed Technology Co,Ltd  - https://www.seeed.cc
#Author: Baozhu Zuo <zuobaozhu@gmail.com>#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License version 2 as
#published by the Free Software Foundation.
#
# no irq version

_uname_r := $(shell uname -r)
_BUILD   := /lib/modules/$(_uname_r)/build
_PWD     := $(shell pwd)
_DEPMOD  ?= depmod -a

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language
ifneq ($(KERNELRELEASE),)


obj-m		+= w1-gpio.o
else
_class  := drivers/input

DEST  := /lib/modules/$(_uname_r)/kernel

all:
	make -C ${_BUILD} M=$(_PWD) modules

clean:
	make -C ${_BUILD} M=$(_PWD) clean

install:
	cp w1-gpio.ko ${DEST}/${_class}/
	$(_DEPMOD)
	@echo w1-gpio.ko >> /etc/modules
uninstall:
	rm -f ${DEST}/${_class}/w1-gpio.ko 
	$(_DEPMOD)
	
.PHONY: all clean install uninstall
endif
