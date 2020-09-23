#!/bin/bash

echo "*********************************************************************************"
echo "*Copyright,       embedfire"
echo "*FileName:        ds18b20"
echo "*Author:          jason"
echo "*Version:         v1.0"
echo "*Date:            20200708"
echo "*Description:     温度传感器测试"
echo "*Others:          按下CTRL+c推出测试"
echo "*********************************************************************************"

while true
do

cat /dev/ds18b20

done

