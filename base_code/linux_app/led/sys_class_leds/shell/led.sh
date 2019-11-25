#RGB灯脚本
#此脚本使用/bin/bash来解释执行

#!/bin/bash

function led1(){
	echo $1 > /sys/class/leds/red/brightness
}

function led2(){
	echo $1 > /sys/class/leds/green/brightness
}

function led3(){
	echo $1 > /sys/class/leds/blue/brightness
}


function led_red(){
	led1 255
	led2 0
	led3 0
}

function led_green(){
	led1 0
	led2 255
	led3 0
}

function led_blue(){
	led1 0
	led2 0
	led3 255
}

keep_alive () {
	while : ; do
		sleep 1
		echo "led_demo is running. Press Ctrl+C to exit!"
	done
}
keep_alive & 
while true
do 
	led_red
	sleep 1
	led_green
	sleep 1
	led_blue
	sleep 1
done















