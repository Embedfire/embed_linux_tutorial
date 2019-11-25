# 输入检测示例

输入检测示例，开发板默认的出厂镜像按键可以使用input子系统驱动进行控制。

各个目录的演示说明如下：
1. c:使用系统调用的文件操作方式控制蜂鸣器
2. shell:使用shell命令控制蜂鸣器

由于主机的Ubuntu系统通常没有GPIO设备，所以这些示例不能在主机上运行。
在配套的ARM开发板可以运行

## c范例说明
使用系统open、read、write、close、ioctl等底层函数读取输入设备事件。

### 编译


编译的输出目录为`build_xxx/input_demo`文件
切换至Makefile所在目录,使用如下命令编译:

x86平台编译指令：
```bash
#注意编译前要把代码中的设备路径修改成主机上的设备
make
```

ARM平台编译指令：
``` bash
make ARCH=arm
```

### 运行

程序默认使用开发板中的KEY按键进行输入测试，运行时可使用参数指定要测试的输入设备路径。

可使用evtest查看系统中存储的输入设备：

```bash
sudo evtest
#以下为示例输出
#No device specified, trying to scan all of /dev/input/event*
#Available devices:
#/dev/input/event0:	Power Button
#/dev/input/event1:	Sleep Button
#/dev/input/event2:	AT Translated Set 2 keyboard
#/dev/input/event3:	Video Bus
#/dev/input/event4:	ImExPS/2 Generic Explorer Mouse
#/dev/input/event5:	VirtualBox USB Tablet
#/dev/input/event6:	VirtualBox mouse integration

#示例中的“VirtualBox mouse integration”为鼠标设备，它的路径为“/dev/input/event6”,
#把“/dev/input/event6”作为本程序的输入参数运行即可，运行后移动鼠标即会有输出。

#如：  sudo ./build_x86/input_demo  /dev/input/event6
```

编译后在Ubuntu上使用如下命令运行：
``` bash
#在Ubuntu对应的目录下运行
#要使用sudo运行！
#要使用sudo运行！

#命令要根据要测试的设备修改，如前面示例中的/dev/input/event6
sudo ./build_x86/input_demo  要测试的设备路径
```

程序默认使用开发板中的KEY按键进行输入测试，运行时也可使用参数指定要测试的输入设备路径。
编译后在ARM终端上使用如下命令运行：
``` bash
#在ARM终端对应的目录下运行
#默认使用KEY按键测试
./build_arm/input_demo
#运行后按开发板上的按键后会有输出

#也可以使用参数指定设备事件路径
./build_arm/input_demo   [要测试的事件设备路径]
#使用相应的设备进行输入
```

--------


## shell范例
直接调用evtest工具检测输入设备

### 运行

脚本本身可能没有可执行权限，需要添加权限后再运行，使用如下命令:
```bash
#在ARM终端对应的脚本目录下执行如下命令
#添加可执行权限
chmod u+x beep.sh
#运行
./input.sh
```
