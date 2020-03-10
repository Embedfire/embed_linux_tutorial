# Makefile步步为营
本目录主要包含Makefile一步步递进学习的示例代码


## step0:Makefile小实验
进行Makefile小实验，体验Makefile的功能。
目录中的`Makefile`是不严谨的编写方式。
目录中的`Makefile_PHONY`是严谨的编写方式。

### make实验

切换至step0目录下，即Makefile所在的同级目录,使用如下命令实验:
``` bash
#在主机上Makefile所在的目录执行如下命令
#查看当前目录的内容
ls
#执行make命令，make会在当前目录下搜索"Makefile"或"makefile"，并执行
make
#可看到make命令后的输出，它执行了Makefile中编写的命令
#查看执行make命令后的目录内容，多了test.txt文件
ls
#执行Makefile的targetd目标，并查看，少了test.txt文件
make   targetd
ls
#执行Makefile的targetb目标，并查看，又生成了test.txt文件
make  targetb
ls
#执行Makefile的targetc目标
make  targetc
```

make使用` -f `参数可以指定使用其它文件。

``` bash
#通过-f选项指定make使用的Makefile文件
make -f Makefile_PHONY
#本示例中其它操作与Makefile功能相同

```

-------


## step1:使用Makefile编译程序
使用Makefile编译Hello示例程序。

### 编译
编译的输出为`hello_main`文件
切换至step1目录下，即Makefile所在的同级目录,使用如下命令编译:
``` bash
#后编译
make
```

### 运行

编译后使用如下命令运行：
``` bash
./hello_main
```

----------

## step2:使用Makefile默认编译规则
使用Makefile编译Hello示例程序，Makefile中使用o文件依赖

### 编译及运行
本实验编译及运行操作与step1相同，切换至step2目录操作即可。

----------

## step3:使用变量
使用Makefile编译Hello示例程序，Makefile中使用变量

### 编译及运行
本实验编译及运行操作与step1相同，切换至step3目录操作即可。

### Makefile_test实验
Makefile_test文件用于变量赋值实验测试，可执行如下命令实验：

```bash
make -f  Makefile
#实验输出为Makefile中各个变量的值
```

----------


## step4:使用变量及分支
使用Makefile编译Hello示例程序，Makefile中使用变量定义最终目标，并使用分支支持不同的架构

### x86架构编译及运行
切换至step4目录下，即Makefile所在的同级目录,使用如下命令编译:
``` bash
#先清理，否则可能因为*.o文件架构不同而出错
make clean
#后编译
make
```

编译后使用如下命令运行：
``` bash
./hello_main
```

### ARM架构编译及运行
切换至step4目录下，即Makefile所在的同级目录,使用如下命令编译:
``` bash
#先清理，否则可能因为*.o文件架构不同而出错
make clean
#后编译
make ARCH=arm
```

编译后使用如下命令运行：
``` bash
./hello_main
```

-------

## step5:使用函数
使用Makefile编译Hello示例程序，Makefile中使用函数，并支持层级目录。

### x86架构编译及运行
切换至step5目录下，即Makefile所在的同级目录,使用如下命令编译:
``` bash
make
```
编译的输出目录为`bulid_x86`

编译后使用如下命令运行：
``` bash
.bulid_x86/hello_main
```

### ARM架构编译及运行
切换至step5目录下，即Makefile所在的同级目录,使用如下命令编译:
``` bash
make ARCH=arm
```
编译的输出目录为`bulid_arm`

编译后使用如下命令运行：
``` bash
.bulid_arm/hello_main
```
### 清理文件
若要清理文件，也要指定架构

1. 清理x86的编译输出，默认架构为x86,直接make clean即可：
  ```bash
  make clean
  ```

2. 清理arm的编译输出，需要使用ARCH=arm指定：
  ```bash
  make clean ARCH=arm
  ```

3. 清理所有的编译输出，可以直接使用cleanall目标：
  ```bash
  make cleanall
  ```






