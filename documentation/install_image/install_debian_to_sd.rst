.. vim: syntax=rst

SD卡烧录Debian镜像
---------------------
本章节讲解如何把Debian镜像烧录到SD卡，如果想把镜像烧录至板载eMMC或NAND FLASH，也要先制作这个SD卡镜像，从SD卡启动开发板。然后再参考下一章节，使用SD卡镜像系统中的fire-config工具刷机到eMMC或NAND FLASH。

Etcher工具简介
~~~~~~~~~~~~~~~~~

Etcher是一款简易好用的镜像烧录工具，主要用于刻录镜像到USB设备，包括SD卡和U盘，
支持ios、img、zip、raw、sdcard等多种镜像格式。它的用户界面清爽美观，操作简单，
在软件自动识别到USB设备后，点击几次鼠标就能完成镜像的烧录。

同时Etcher是一个跨平台工具，支持 Mac、Windows 和 Linux ，
国内外开发者常用它将树莓派SD卡启动镜像烧录到SD卡中。

下载安装Etcher工具
~~~~~~~~~~~~~~~~~~~~~~

打开Etcher官网地址: \ https://www.balena.io/etcher/\,
在官网首页有Etcher工具的下载链接，用户可以根据自己的操作系统类型，来下载对应版本的软件安装包。

..  image:: media/instal002.png
    :align: center
    :alt: Etcher工具

下载完成后，直接安装软件即可。

烧录Debian镜像
~~~~~~~~~~~~~~~~~~~~~

下载野火最新发布的Debian系统镜像：\ https://pan.baidu.com/s/1pji1U1vvgP8sl_TTZp59qg/\。(提取码:b5wb)

打开安装好的Etcher工具，软件界面如下图所示:

..  image:: media/instal003.png
    :align: center
    :alt: Etcher工具



选择要烧录的镜像，野火提供的Debian镜像三个不同版本，如下图所示:

..  image:: media/install_debian3.png
    :align: center
    :alt: Debian镜像

其中:

- Debian Buster Lite目录下存放的是Debian的纯净版固件，没有带桌面环境和野火的QT demo。

- Debian with desktop目录下存放的是带有桌面环境的Debian固件，系统启动后会进入桌面环境。

- Debian with QT_demo目录下存放的带有野火综合QT demo的Debian固件，系统启动后会进入QT demo的界面。


以纯净版固件为例，进入Debian Buster Lite目录，可以看到Debian镜像是一个img格式的文件，
可以直接使用Etcher工具进行烧录。如下图所示:

..  image:: media/install_debian4.png
    :align: center
    :alt: Debian镜像

选择好Debian镜像后，继续选择要烧录的SD卡，然后点击"flash"按钮开始进行烧录。

..  image:: media/instal005.png
    :align: center
    :alt: Debian镜像

烧录过程会显示镜像的烧录进度，如下图

..  image:: media/instal006.png
    :align: center
    :alt: Debian镜像

等到软件出现出现如下界面时，表示镜像已经成功烧录完毕。

..  image:: media/install_debian7.png
    :align: center
    :alt: Debian镜像

开发板插入烧录好Debian镜像的SD卡，调整拨码开关为SD卡启动方式，上电后开发板就能正常启动。

注意:开发板第一次从SD卡启动时，系统会自动进行扩容重启，
以保证充分利用SD卡存储空间。扩容完毕后，系统重启生效。

系统启动以后，输入用户密码登录即可：

普通用户:

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

   账户:debian
   密码:temppwd

ROOT用户:

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

   账户:root
   密码:root
