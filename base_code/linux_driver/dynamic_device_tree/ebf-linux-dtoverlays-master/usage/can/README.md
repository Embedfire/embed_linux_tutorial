#
## 安装
```bash
sudo apt-get install can-utils
```


#copy 'cansend' & 'candump' & 'ip.iproute'
cp /mnt/b/usr/bin/cansend /usr/bin/
cp /mnt/b/usr/bin/candump /usr/bin/
cp /mnt/b/sbin/ip.iproute2 /sbin/


cd /sbin/
ln -sf ip.iproute2 ip


##  使用方法
设置波特率是1M，启动can
```bash

ip link set can0 type can bitrate 1000000;ip link set can0 up

ip link set can1 type can bitrate 1000000;ip link set can1 up
```



发送数据
关于cansend的用法，可以通过参数h。
```
cansend can0 123#abcdabcd
```

#接收数据
```bash
candump can0
```
