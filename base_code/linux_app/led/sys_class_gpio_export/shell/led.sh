#RGB灯脚本
#此脚本使用/bin/sh来解释执行

#!/bin/bash

while true
do
	echo "=================LED test,Press q to exit================="
	echo "Please Input the RGB index : 4/116/115(4--R 116--G 115--B)"
	#等待输入灯的编号
	read index      
	#输入有效性检测	                                                                                                                 	
	case $index in
	4) 
	;;
	115) 
	;;
	116) 
	;;
	q) echo "quit"	
       exit 0   
	;;
	*) echo "Incorrect format input"
	   continue
	;;
	esac

	while true
	do
		echo "Please Input the value : 0-on 1-off"
		#输入LED的值
		read value	
		#输入有效性检测		
		case $value in
		0) break
		;;
		1) break
		;;
		q) echo "quit"
		   exit 0
		;;
		*) echo "Incorrect format input"
		   continue
		;;
		esac
	done

	#是否初始化LED灯
	if [ ! -e /tmp/led$index ]
	 then 
	 echo "init led hardware" > /tmp/led$index
	 #初始化相应的GPIO口
	 echo $index > /sys/class/gpio/export
	 #设置GPIO口的方法，由于是用于点亮RGB灯，因此设置为输出模式
	 echo 'out' > /sys/class/gpio/gpio$index/direction
	fi

	#控制LED灯的亮灭。
	echo $value > /sys/class/gpio/gpio$index/value 
done
