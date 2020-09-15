
.. vim: syntax=rst

驱动章节实验环境搭建
==============================

引言
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
本章主要目的是搭建驱动章节的实验环境，方便后续章节不在实验环境上耗费太多时间，
而是集中精力理解设备驱动的原理。

首先我们要明白程序最终是运行在开发板上，我们开发板主要使用i.MX6ULL系列处理器，
它包含一个Cortex-A7(ARM的一种高能效处理器架构)内核。开发板上已经移植好相关的环境，
我们只需要将我们写的代码交叉编译成arm架构下的可执行文件即可。

设备驱动是具有独立功能的程序，它可以被单独编译，但不能独立运行，
在运行时它被链接到内核作为内核的一部分在内核空间运行。也因此想要我们写的内核模块在某个版本的内核上运行，
那么就必须在该内核版本上编译它，如果我们编译的内核与我们运行的内核具备不相同的特性，设备驱动则无法运行。

首先我们需要知道内核版本，并准备好该版本的内核源码，
使用交叉编译工具编译内核源码，最终再依赖编译好的内核编译我们的程序。

环境准备
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
安装工具
------------------------------
在编译源码之前我们需要先准备好交叉编译的环境，安装必要的依赖和工具，

- gcc-arm-linux-gnueabihf  交叉编译器
- bison 语法分析器
- flex  词法分析器
- libssl-dev OpenSSL通用库
- lzop LZO压缩库的压缩软件

执行下面的命令即可：

::

   sudo apt install make gcc-arm-linux-gnueabihf gcc bison flex libssl-dev dpkg-dev lzop

编译内核
------------------------------
开发板内核使用 Linux npi 4.19.71-imx-r1 版本，可以使用命令'uname -a'查看。
我们可以从github或者gitee上克隆开发板的Debian镜像内核源码，
国内推荐使用gitee下载，当然首先需要安装git工具。

github:

::

   git clone https://gitee.com/Embedfire/ebf-buster-linux.git

gitee:

::

   git clone https://gitee.com/Embedfire/ebf-buster-linux.git

设备驱动被链接到内核有两种方式，编译成模块和直接编译进内核；

首先就单独编译内核，再将设备驱动编译成模块来讲：

我们可以单独新建一个工作目录，将内核源码放置在该目录下，切换到内核源码目录，我们可以找到make_deb.sh脚本，
里面有配置好的参数，只需要执行便可编译内核。编译出来的内核相关文件存放位置，
由脚本中的ebf-buster-linux/make_deb.sh中build_opts="${build_opts} O=build_image/build" 指定。

.. image:: media/exper_env001.png
   :align: center
   :name: 内核模块信息

.. image:: media/exper_env002.png
   :align: center
   :name: 内核模块信息

接下来我们不妨简单了解一些内核的构建原理。

内核的构建原理
------------------------------
首先是make_deb.sh脚本

.. code:: bash

   deb_distro=bionic
   DISTRO=stable
   build_opts="-j 6"
   #指定编译好的内核放置位置
   build_opts="${build_opts} O=build_image/build"
   #编译出来的目标是针对ARM体系结构的内核
   build_opts="${build_opts} ARCH=arm"
   #对于deb-pkg目标，允许覆盖deb-pkg部署的常规启发式。
   build_opts="${build_opts} KBUILD_DEBARCH=${DEBARCH}"
   #使用内核配置选项“LOCALVERSION”为常规内核版本附加一个唯一的后缀。
   build_opts="${build_opts} LOCALVERSION=-imx-r1"

   build_opts="${build_opts} KDEB_CHANGELOG_DIST=${deb_distro}"
   build_opts="${build_opts} KDEB_PKGVERSION=1${DISTRO}"
   #指定交叉编译器
   build_opts="${build_opts} CROSS_COMPILE=arm-linux-gnueabihf-" 
   build_opts="${build_opts} KDEB_SOURCENAME=linux-upstream"
   make ${build_opts}  npi_v7_defconfig
   make ${build_opts}  
   make ${build_opts}  bindeb-pkg


我们可以去https://www.kernel.org/doc/html/latest/index.html 和 https://www.debian.org/ 了解更多关于


编译程序
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
获取内核模块示例源码，将配套代码解压到内核代码同级目录，这里以Linux内核模块章节的hellomodule实验为例。
实验代码位于：/base_code/linux_driver/module/hellomodule。

github:

::

   git clone https://github.com/Embedfire-imx6/embed_linux_tutorial

gitee:

::

   git clone https://gitee.com/Embedfire-imx6/embed_linux_tutorial

在内核源码外编译
------------------------------
内核模块对象所需的构建步骤和编译很复杂，它利用了linux内核构建系统的强大功能，
当然我们不需要深入了解这部分知识，利用简单的Make工具就能编译出我们想要的内核模块。
::

   cd hellomodule
   make

.. image:: media/exper_env003.jpg
   :align: center
   :name: 实验环境

.. image:: media/exper_env004.jpg
   :align: center
   :name: 实验环境

注意该目录下的Makefile中 "KERNEL_DIR=../ebf-buster-linux/build_image/build"要与前面编译的内核所在目录一致。

.. code:: bash

   #指定编译内核存放位置
   KERNEL_DIR=../../ebf-buster-linux/build_image/build
   #针对ARM体系结构
   ARCH=arm
   #交叉编译工具链
   CROSS_COMPILE=arm-linux-gnueabihf-
   #导入环境变量
   export  ARCH  CROSS_COMPILE
   #表示以模块编译
   obj-m := hellomodule.o
   #all只是个标号，可以自己定义，是make的默认执行目标。
   all:
      $(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

   .PHONE:clean copy

   clean:
      $(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean	


$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
$(MAKE):MAKE是Makefile中的宏变量，要引用宏变量要使用符号。这里实际上就是指向make程序，
所以这里也可以把$(MAKE)换成make.-C:是make命令的一个选项，-C作用是changedirectory. 
-C dir 就是转到dir目录。M=$(CURDIR)：返回当前目录。

这句话的意思是：当make执行默认的目标all时，-C(KVDIR)指明跳转到内核源码目录下去执行那里的Makefile,
-C $(KERNEL_DIR)指明跳转到内核源码目录下去执行那里的Makefile,M=(CURDIR)表示又返回到当前目录来执行当前的Makefile.

clean 就是删除后面这些由make生成的文件。

查看文件夹，新增hellomodule.ko，这就是我们自己编写、编译的内核模块。
使用file hellomodule.ko查看当前编译的文件，32-bit ARM架构的ELF文件。
::

   file hellomodule.ko
   hellomodule.ko: ELF 32-bit LSB relocatable, ARM, EABI5 version 1 (SYSV),
   BuildID[sha1]=1a139278874b2e1a335f1834e755d2cf3f9a4bff, not stripped



和内核源码一起编译
------------------------------