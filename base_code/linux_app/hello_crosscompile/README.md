# 使用C语言编写Hello world程序
本目录主要包含使用C语言编写的输入Hello world的程序。

## 使用make编译
切换至Makefile所在目录。
使用如下命令编译:
``` bash
make
```

## 直接使用编译器编译
切换至hello.c所在目录。
使用如下命令编译:
``` bash
arm-linux-gnueabihf-gcc -o hello hello.c
```



## 运行

编译后`在开发板的终端上，切换至共享目录`，使用如下命令运行：
`以下命令在开发板的终端上运行`
`以下命令在开发板的终端上运行`
``` bash
#切换至程序所在目录运行
./hello
```

