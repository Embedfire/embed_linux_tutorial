#!/bin/sh

rm COPYING
rm COPYING.preface
rm config-pin.c

wget https://raw.githubusercontent.com/pmunts/muntsos/master/COPYING
wget https://raw.githubusercontent.com/pmunts/muntsos/master/bootkernel/initramfs/COPYING.preface
wget https://raw.githubusercontent.com/pmunts/muntsos/master/bootkernel/initramfs/config-pin.c
