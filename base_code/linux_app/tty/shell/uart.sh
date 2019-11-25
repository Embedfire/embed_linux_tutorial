#!/bin/bash
# 串口(uart3)发送接受实验
# echo用于向串口发送数据
# cat用于接收数据
# 按“Ctrl+C”可退出
# 注意：用户往UART3发送数据时，需要在发送内容后加上换行符
echo "embedfire imx6ull uart demo" > /dev/ttymxc2

echo "Waiting for ttymx2 input..."
cat /dev/ttymxc2
