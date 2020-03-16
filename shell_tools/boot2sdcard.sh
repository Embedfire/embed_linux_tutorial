#!/bin/bash

if [ -e *.dtb ]; then
	if [ ! -e imx6ull-14x14-evk.dtb ]; then
		echo "*.dtb exist, but not 'imx6ull-14x14-evk.dtb'"
		echo "Starting backup..."
		var=`ls *.dtb`
		cp $var imx6ull-14x14-evk.dtb
		mv $var $var.bk
	fi
fi



while true
do
    sudo fdisk -l
    echo -e "\n\033[31m请确保输入的sd卡编号正确！！！\n\033[0m"
    read -p "Please Input the card ID [a~z] (Input 'exit' for quit): " dev_index
    # if [ $dev_index == "exit" ]
    #     then
    #     exit 0;
    # fi
    case $dev_index in  
    [[:lower:]]) break
    ;;
    exit) exit 0
    ;;

    * ) continue
    ;;

    esac  
done



sd_idnex=sd$dev_index
sd_first=${sd_idnex}1
sd_sec=${sd_idnex}2

sudo fdisk /dev/$sd_idnex << EOF
n
p
1
20480
1024000
n
p
2
1228800

w
EOF


sudo dd if=$1 of=/dev/$sd_idnex bs=512 seek=2 conv=fsync

# 复制内核和设备树 
mkdir  tmp

sudo mkfs.vfat /dev/${sd_first}

sudo mount /dev/$sd_first tmp

sudo cp $2  tmp
sudo cp imx6ull-14x14-evk.dtb tmp
sudo umount tmp

# 复制根文件系统
sudo mkfs.ext4 /dev/$sd_sec

sudo mount /dev/$sd_sec tmp

sudo tar -jxvf $4 -C tmp

sudo umount tmp



#删除临时文件夹
rm -rf tmp
