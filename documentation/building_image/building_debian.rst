.. vim: syntax=rst

构建野火Debian系统固件
------------------------

前面介绍了NXP官方的固件有稳定可靠的优点，适合大部分的产品使用，但这是从产品角度来看的。
对于Linux学习者来说，原厂固件因为使用的是较老版本的linux内核、Uboot版本，
它与目前的代码的架构和系统特性已经发生了一些变化。再去学习研究时，就显得比较脱节。
毕竟学习时间有限，不可能把所有新旧版本的内核、uboot重新学习一遍。

目前野火移植了较新版本的uboot(2019_04) ,kernel(4.19.71)，为的就是减少重复学习成本，
也更加锻炼学习者的能力。另外,出于对初学者好学易用的目的，
根文件系统将会使用Debian，它的第三方软件包繁多，安装简单，
大家熟悉的Ubuntu就是基于Debian发展而来。


编译2019.04版本uboot
============================

野火移植2019.04版本uboot，在其中完善了对内核设备树插件的支持，提高了系统的扩展性和可维护性。

1、下载野火2019版本uboot，代码已经托管在github上，直接执行以下命令进行下载即可:

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   git clone https://github.com/Embedfire/ebf-buster-uboot.git

2、检查交叉编译工具链是否正常

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   arm-none-eabi-gcc -v
   # 输出
   gcc version 6.3.1 20170620 (15:6.3.1+svn253039-1build1)

如果系统还没安装交叉编译工具链，使用以下命令进行安装:

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   sudo apt-get install gcc-arm-none-eabi

3、在项目文件夹目录下使用root权限执行编译脚本compile_uboot.sh

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   sudo ./compile_uboot.sh

编译结束后，会在当前文件夹路径下生成u-boot-dtb.imx文件，这就是我们需要的uboot镜像。

编译4.19.71版本内核
=========================

1、下载野火4.19.71版本内核，代码已经托管在github上，直接执行以下命令进行下载即可:

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   git clone https://github.com/Embedfire/ebf-buster-linux.git

2、检查交叉编译工具链是否正常

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   arm-none-eabi-gcc -v
   # 输出
   gcc version 6.3.1 20170620 (15:6.3.1+svn253039-1build1)

如果系统还没安装交叉编译工具链，使用以下命令进行安装:

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   sudo apt-get install gcc-arm-none-eabi

3、在项目文件夹目录下使用root权限执行编译脚本make_deb.sh

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

   sudo ./make_deb.sh

编译结束后，会在/home/pi路径下生成linux-image-4.19.71-imx-r1_1stable_armhf.deb，这就是4.19.71版本linux内核的安装包。


制作Debian系统镜像
=============================

ebf-image-builder简介
~~~~~~~~~~~~~~~~~~~~~~~

该项目移植自BeagleBone公司的image-builder项目，主要用于构建debian系统镜像。它的源码开放，
主要由shell脚本和配置文件构成，用户可以灵活高效地修改debian文件系统的配置，
还能根据实际项目需要在文件系统中预装某些应用软件。

源码地址: https://github.com/Embedfire/ebf-image-builder

关于Debian系统
~~~~~~~~~~~~~~

Linux有非常多的发行版本，Debian就是最早的Linux发行版本之一。大家比较熟悉的Ubuntu就是基于Debian发展而来，
相比其他Linux发型版本，Debian主要有以下几个方面的优点:

1.  稳定

Debian的发布版本通常测试完善，发行周期较长，它通常维护着三个版本:"稳定版(stable)"、"测试版(testing)"、
"不稳定版(unstable)"，对每个稳定发行版本，用户可以得到三年的完整支持以及额外两年的长期支持。
debian目前在很多企业用户中使用，它的稳定性和可靠性是经过市场长期验证的。

2.  软件包管理程序简单易用

熟悉Ubuntu的人应该知道，Ubuntu是用dpkg工具来进行软件包的管理，关于软件的安装、卸载、升级都可以用dpkg指令完成，
dpkg指令功能非常强大，但是也比较复杂。因而后来基于dpkg衍生出来了apt工具，可以通过apt install、apt remove、
等指令就能轻松安装、卸载软件。在这方面，Debian与Ubuntu的机制是完全一样的。

3.  软件包丰富

Debian经过这么多年的发展，开源社区已经积累了数以万计的应用程序，从文档编辑、到电子商务、到游戏娱乐、到软件开发，
全面提供即安装即使用的体验。免去自己编译源代码的诸多麻烦，而且用apt upgrade就能轻松升级到最新版本的软件。

4.  良好的系统安全

Debian自由软件社区非常注重在软件发布中快速地修复安全问题。通常没几天就会有修复过的软件被上传。因为开放源代码
所以 Debian 的安全性是可以被用户和开发者客观评估的。这有利于防止潜在的安全问题被引入到Debian系统中。



关于image-builder项目
~~~~~~~~~~~~~~~~~~~~~

项目起源
""""""""

BeagleBoard.org是一家非盈利公司，致力于在嵌入式计算 领域提供开源硬件和软件，并且重点关注教育行业,
BeagleBoneBlack是该公司最受欢迎的主板。image-builder项目就是为BeagleBoneBlack之类的开发板构建debian系统镜像的开源项目。

源码地址: https://github.com/beagleboard/image-builder。

项目原理
"""""""""

image-builder项目基于qemu和debootstrap等工具来生成和定制arm架构的debian文件系统。其中，qemu是一个开源的模拟器项目，
在GNU/Linux平台上使用广泛，可用于模拟各种不同架构的处理器。而debootstrap是Debian官方提供的，
适用于生成不同架构、不同版本的debian文件系统。

.. image:: media/image-builder_analyze.png
   :align: center
   :alt: image-builder项目分析

如上图所示:
在使用debootstrap工具生成debian文件系统后，然后qemu使用新构建的debian文件系统作为文件系统，
在qemu模拟arm架构的处理器并启动后，就可以用apt install命令预装各种应用软件，
linux内核和设备树都是在这个阶段中安装更新的。安装完毕后，使用dd命令把得到的文件系统和u-boot一起打包到img文件中，
即可生成用于sd卡烧录的debian系统镜像。

ebf-image-builde目录分析
~~~~~~~~~~~~~~~~~~~~~

1.  使用git克隆项目镜像

.. code-block:: sh
   :emphasize-lines: 1
   :linenos:

    git clone https://github.com/Embedfire/ebf-image-builder.git

出现下图提示表示克隆完成

.. image:: media/git_clone_finish.png
   :align: center
   :alt: git克隆完成

2.  进入下载好的文件目录，使用tree命令查看文件夹目录

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

   cd ebf-image-builder
   tree -L 1

可以看到ebf-image-builde项目的目录结构，如下图所示:

.. image:: media/ebf-image-builder_list.png
   :align: center
   :alt: ebf-image-builde目录

其中:

- configs目录:主要是Debian文件系统的一些配置项，主要有:预安装软件列表、镜像源地址等。
- docs目录:主要是代理使用说明和debootstrap使用说明。
- publish目录:这个目录下存放着编译debian固件的起始脚本，是整个项目的编译入口。
- scripts目录:存放了一些执行特殊功能的脚本，在编译过程中会被调用。
- target目录:这里面放置的内容比较杂散，有启动参数的说明文档、systemd的启动服务和deb镜像源的密钥等等。
- tools目录:主要是打包脚本，完成把文件系统和uboot打包成img镜像的功能。
- uboot目录:放置的是uboot固件。

ebf-image-builde编译Debian固件
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

进入ebf-image-builde项目源码目录下，直接执行编译脚本

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

   cd ebf-image-builder
   sudo ./publish/seeed-imx-stable.sh

注意：如果执行过程提示:

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

    m4: 未找到命令

那么先用apt工具安装m4工具，再重新执行编译命令

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

   sudo apt install m4 -y
   sudo ./publish/seeed-imx-stable.sh

正常编译时，打印信息如下图所示:

.. image:: media/building_debian_start.png
   :align: center
   :alt: 开始编译debian

编译时间较长(大概三十分钟到一个小时不等，主要跟网速有关)，请耐心等待。如果后面客户需要频繁进行编译工作，
我们会提供使用代理下载的方法，可大幅减小编译时间。

编译完成后，下图中红框部分即为新编译的Debian系统镜像

.. image:: media/building_debian_end.png
   :align: center
   :alt: 编译debian结束

Debian系统镜像存放下面目录中

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

    ebf-image-builder/deploy/debian-buster-console-armhf##日期 
    
可以参考《SD卡烧录Debian镜像》章节把该镜像烧录到sd卡中，以SD卡方式启动开发板。

烧录完成后，开发板第一次用sd卡方式启动时，系统会自动进行扩容重启，
以保证充分利用sd卡存储空间。扩容完毕后，系统重启生效。

