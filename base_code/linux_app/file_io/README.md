# 文件IO操作
本目录主要包含`stdio`和`systemcall`文件操作的范例。


## stdio:标准C库方式
使用C库的fopen、fread、fwrite、fclose、fioctl等库函数。
这种方式通常用于访问存储器的文件或终端输入输出。

### 编译
编译的输出目录为`build_xxx/file_demo`文件
切换至Makefile所在目录,使用如下命令编译:
``` bash
#x86架构
make
```

ARM平台编译指令：
``` bash
#arm架构
make ARCH=arm
```

### 运行

编译后使用如下命令运行：
``` bash
#x86
./build_x86/file_demo
#ARM
./build_arm/file_demo
```
-------


## systemcall:系统调用方式
使用系统open、read、write、close、ioctl等底层函数。
这种方式通常用于访问设备驱动程序。

### 编译和运行
本示例的编译和运行与stdio的相同，切换至相应目录以同样的方式操作即可


