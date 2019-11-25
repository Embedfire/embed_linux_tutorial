# LED点灯示例
LED示例，硬件相关的示例依赖于设备树。
在应用层点亮LED灯有`/sys/class/gpio_export`和`/sys/class/leds`两种方式，
受设备树的影响，本开发板默认**只能**使用`/sys/class/leds`控制板上RGB和心跳灯。
默认情况下，使用`/sys/class/gpio_export`**无法**对板载LED灯进行控制。

本目录下的示例使用`/sys/class/gpio_export`的LED设备，默认不能正常运行，在以后学习了替换设备树可更换LED灯的控制方式。

各个目录的演示说明如下：
1. c_systemcall:使用系统调用的文件操作方式点灯
2. shell:使用shell命令点灯

由于主机的Ubuntu系统没有LED设备，所以这些示例不能在主机上运行。
在配套的ARM开发板可以运行

## c_systemcall范例说明
使用系统open、read、write、close、ioctl等底层函数控制LED设备。

### 编译


编译的输出目录为`build_xxx/file_demo`文件
切换至Makefile所在目录,使用如下命令编译:
ARM平台编译指令：
``` bash
make ARCH=arm
```

### 运行
Ubuntu主机通常没有LED设备，本实验不支持在主机上运行。
编译后在ARM终端上使用如下命令运行：
``` bash
#在ARM终端对应的目录下运行
./build_arm/led_demo
```


-------

## shell范例
使用echo等脚本控制LED设备。


## 运行
Ubuntu主机通常没有LED设备，本实验不支持在主机上运行。

脚本本身可能没有可执行权限，需要添加权限后再运行，使用如下命令:
```bash
#在ARM终端对应的脚本目录下执行如下命令
#添加可执行权限
chmod u+x led.sh
#运行
./led.sh
```
