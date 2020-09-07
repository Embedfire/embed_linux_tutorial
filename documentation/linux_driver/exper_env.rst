驱动模块实验环境搭建
^^^^^^^^^^^^^^^^^^^^^^^^
理清楚代码的基本结构，我们就要构建环境来运行我们的代码了，
如果想要我们写的内核模块在某个版本的内核上运行，那么就必须在该内核版本上编译它，
如果我们编译的内核与我们运行的内核具备不相同的特性，那么编译生成的内核模组就无法运行，
为了严谨起见，我们编译的内核模组要在开发板上运行，我们就需要知道开发板的内核版本，
使用'uname -r'命令可以查看对应的内核版本。

同时我们可以从github或者gitee上克隆开发板的Debian镜像内核源码，国内推荐使用gitee下载，当然首先需要安装git工具。
github:
::

   git clone https://gitee.com/Embedfire/ebf-buster-linux.git

gitee:
::

   git clone https://gitee.com/Embedfire/ebf-buster-linux.git

驱动进入内核有两种方式，编译成模块和直接编译进内核；
前面我们就了解到了模块是具有独立功能的程序，它可以被单独编译，但不能独立运行，
在运行时它被链接到内核作为内核的一部分在内核空间运行，而它在编译的时候也必须依赖
内核，所以我们有必要在编译内核模块之前先编译内核。


编译内核
:::::::::::::::::::::::::::
在编译源码之前我们需要先安装如下工具，执行后面的脚本即可：

   - gcc-arm-linux-gnueabihf  交叉编译器
   - bison 语法分析器
   - flex  词法分析器
   - libssl-dev OpenSSL通用库
   - lzop LZO压缩库的压缩软件

::

   sudo apt install make gcc-arm-linux-gnueabihf gcc bison flex libssl-dev dpkg-dev lzop

切换到内核源码目录下，我们可以找到make_deb.sh脚本，里面有我们配置好的参数，只需要执行便可编译内核。
编译出来的内核相关文件位置，由脚本中的ebf-buster-linux/make_deb.sh中build_opts="${build_opts} O=build_image/build" 指定。

.. image:: media/module002.png
   :align: center
   :alt: 内核模块信息

接下来我们不妨简单了解一些内核的构建原理。

内核的构建原理
'''''''''''''''''''''''''''
首先是make_deb.sh脚本

.. code:: bash

   deb_distro=bionic
   DISTRO=stable
   build_opts="-j 6"
   #指定编译好的内核放置位置
   build_opts="${build_opts} O=build_image/build"
   #编译出来的目标是针对ARM体系结构的内核
   build_opts="${build_opts} ARCH=arm"
   build_opts="${build_opts} KBUILD_DEBARCH=${DEBARCH}"
   build_opts="${build_opts} LOCALVERSION=-imx-r1"
   build_opts="${build_opts} KDEB_CHANGELOG_DIST=${deb_distro}"
   build_opts="${build_opts} KDEB_PKGVERSION=1${DISTRO}"
   #指定交叉编译器为arm-linux-gnueabihf-
   build_opts="${build_opts} CROSS_COMPILE=arm-linux-gnueabihf-" 
   build_opts="${build_opts} KDEB_SOURCENAME=linux-upstream"
   make ${build_opts}  npi_v7_defconfig
   make ${build_opts}  
   make ${build_opts}  bindeb-pkg


编译hellomodule
:::::::::::::::::::::::::::

在内核源码外编译
'''''''''''''''''''''''''''
1.获取内核模块示例源码，将配套代码 /base_code/linux_driver/module/hellomodule 解压到内核代码同级目录

github:
::

   git clone https://github.com/Embedfire-imx6/embed_linux_tutorial

gitee:
::

   git clone https://gitee.com/Embedfire-imx6/embed_linux_tutorial


2.内核模块对象所需的构建步骤和编译很复杂，它利用了linux内核构建系统的强大功能，
当然我们不需要深入了解这部分知识，利用简单的Make工具就能编译出我们想要的内核模块。
::

   cd hellomodule
   make

.. image:: media/module008.jpg
   :align: center

.. image:: media/module009.jpg
   :align: center

注意该目录下的Makefile中 "KERNEL_DIR=../ebf-buster-linux/build_image/build"要与前面编译的内核所在目录一致。
查看文件夹，新增hellomodule.ko，这就是我们自己编写、编译的内核模块。
使用file hellomodule.ko查看当前编译的文件，32-bit ARM架构的ELF文件。
::

   file hellomodule.ko
   hellomodule.ko: ELF 32-bit LSB relocatable, ARM, EABI5 version 1 (SYSV),
   BuildID[sha1]=1a139278874b2e1a335f1834e755d2cf3f9a4bff, not stripped

和内核源码一起编译
'''''''''''''''''''''''''''