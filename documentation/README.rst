.. vim: syntax=rst

关于本项目
==============

本项目通过git开源：

- github仓库地址：https://github.com/Embedfire/embed_linux_tutorial
- gitee 仓库地址：https://gitee.com/Embedfire/embed_linux_tutorial

点击右侧链接可在线阅读本项目文档：《 `野火i.MX Linux开发实战指南 <https://embed-linux-tutorial.readthedocs.io>`_ 》

本书主要面向有MCU开发经验，希望从零开始学习Linux开发的嵌入式软件工程师及在校学生。

目前国内关于嵌入式Linux的教材大都比较老旧，要么是基于多年前的ARM9架构CPU，要么是Linux系统内核还沿用2.6版本的，对于较新的技术如设备树、Yocto编译工具等鲜有介绍。本书希望打破这种局面，配套的硬件平台采用Cortex-A7/A9等架构的CPU，讲解的Linux内核基于4.19版本，
循序渐进且涉及的知识全面，紧跟技术潮流，为学习者建立起Linux开发的全貌。

本书将分为以下几个部分进行讲解。

-  第一部分主要是Linux系统环境，在PC上通过Ubuntu熟悉Linux系统的基本使用。若能熟练使用Linux的用户可以直接跳过。

-  第二部分主要是熟悉ARM Linux，在ARM开发板上的Linux系统上操作，了解与PC上Linux系统的异同，进一步熟悉命令行以及硬件的基本控制。

-  第三部分通过带领读者编写基本的Linux应用初步了解Linux下的开发模式。

-  第四部分引领读者亲手给开发板定制Linux内核、uboot以及文件系统。

-  第五部分讲解Linux的系统编程，使用系统调用进行进程管理、异步操作等。

-  第六部分通过几个简单的裸机程序熟悉imx系列芯片的外设

-  第七部分介绍如何针对开发板编写适用于Linux的驱动程序。

-  第八部分通过项目实战的方式讲解如何在ARM Linux开发板上搭建各类应用，如qt、pyqt、物联网关、can、modbus、opencv等软件框架的使用。


我们特意在第一第二部分通过一些小实验让读者把开发板用起来，就是希望让大家尽快接触开发板，提高学习的热情，而不是让开发板躺着吃灰。

本教程使用的开发环境说明如下：

- PC系统Windows：默认使用Win10 64位，兼容Win7等系统。在使用NXP的MFG工具烧录时需要使用到Windows系统。

- PC系统Linux：**Ubuntu18.04** ，强烈建议使用相同的版本。
  教程讲解了在Windows上使用VirtualBox虚拟机安装的说明，使用其它虚拟机如VMware安装也可以。

- 开发板系统：配套 **Debian镜像** 的 **full-qt-app** 版本，由于开发板出厂烧录的镜像是经裁剪过的，
  所以按教程学习时建议重新烧录镜像到SD卡运行，学习过程会安装较多的软件工具，建议使用4G以上的SD卡。
  镜像下载链接 https://sourceforge.net/projects/ebf-debian-firmware 。
