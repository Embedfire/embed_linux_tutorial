# LED点灯示例

LED示例，硬件相关的示例依赖于设备树。
在应用层点亮LED灯有`/sys/class/gpio_export`和`/sys/class/leds`两种方式，
受设备树的影响，本开发板默认**只能**使用`/sys/class/leds`示例控制板上RGB和心跳灯。
默认情况下，使用`/sys/class/gpio_export`示例**无法**对板载LED灯进行控制。

本目录下的示例使用`/sys/class/leds`的LED设备，默认可以正常运行，使用可确认通过查看是否存在相应的设备文件确认。

各个目录的演示说明如下：
1. c_shell:使用C代码执行shell命令的控制方式，这是比较傻的方式，只是作为参考
2. c_stdio:使用C标准库文件操作方式点灯
3. c_systemcall:使用系统调用的文件操作方式点灯
4. shell:使用shell命令点灯

由于主机的Ubuntu系统没有LED设备，所以这些示例不能在主机上运行。
在配套的ARM开发板可以运行

## c_stdio范例说明
使用库函数fopen、fread、fwrite、fclose、fflush控制led设备。

### 编译


编译的输出目录为`build_xxx/led_demo`文件
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


## c_systemcall范例
使用系统open、read、write、close、ioctl等底层函数控制LED设备。


### 编译和运行
本示例的编译和运行与stdio的相同，切换至相应目录以同样的方式操作即可

-------


## c_shell范例
使用系统system系统调用的方式执行命令行。


### 编译和运行
本示例的编译和运行与stdio的相同，切换至相应目录以同样的方式操作即可

-------

## shell范例
使用echo等脚本控制LED设备。


### 运行
Ubuntu主机通常没有LED设备，本实验不支持在主机上运行。

脚本本身可能没有可执行权限，需要添加权限后再运行，使用如下命令:
```bash
#在ARM终端对应的脚本目录下执行如下命令
#添加可执行权限
chmod u+x led.sh
#运行
./led.sh
```
