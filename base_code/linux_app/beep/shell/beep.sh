#!/bin/bash

echo "=================BEEP test, Press q to exit================="
#蜂鸣器对应的引脚编号
index=19
#系统是否导出了相应的gpio文件夹
if [ ! -e /sys/class/gpio/gpio$index ]
	then
	echo "init hardware" > /tmp/beep

	echo $index > /sys/class/gpio/export 
	echo 'out' > /sys/class/gpio/gpio$index/direction
fi
while true
do
	echo "Please Input the value : 0--off 1--on q--exit"
	#读取端口的输入电平，为1时，蜂鸣器响	
	read value
	case $value in
		0) echo $value > /sys/class/gpio/gpio$index/value
		;;
		1) echo $value > /sys/class/gpio/gpio$index/value
		;;
		q) echo "quit"
		   exit 0
	   	;;
     		*) echo "Incorrect format input"
		;;		
	esac
done
