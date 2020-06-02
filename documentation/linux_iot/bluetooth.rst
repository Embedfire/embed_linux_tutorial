Debian使用蓝牙音箱
==================

本章旨在帮助用户正确使用蓝牙连接音箱，引导用户快速验证相关的功能模块。

-  开发板：野火imx6ull开发板
-  系统版本：Debian系统
-  需要使用的模块/设备：免驱USB蓝牙、蓝牙音箱

注意，此处使用的是\ **免驱USB蓝牙**\ ，注意一定要免驱的，否则开发板识别不了，大家可以自行在淘宝上购买。为什么不使用板载蓝牙呢，因为在测试中发现，蓝牙速率不够，略有卡顿。

开发板本身带有两个音频输出方案：板载外接扬声器和3.5mm耳机接口。而现在各种蓝牙耳机和音箱也越来越多，大多数智能音箱也支持蓝牙连接，将开发板与这些无线音频输出设备进行连接变得流行起来了。本文简单介绍一种方法将蓝牙音箱与开发板连接起来。

方案简介
--------

在进行正式的配置之前，有必要简单介绍一下各个软件包的作用和原理。

ALSA(Advanced Linux Sound
Architechture)是Linux上的主流音频框架，是linux内核的一部分，它是最底层的音频接口，一般只有声卡驱动厂商需要与其对接，普通用户不需要了解这一部分，只要使用即可。后面使用的aplay命令就是ALSA相关的软件包（alsa-utils）提供的。

Bluez是一个linux蓝牙协议栈，简单说就是linux上所有蓝牙相关的底层功能都是由它提供的，没有他蓝牙就无法工作，Bluez主要提供蓝牙基础设施、和最基本的蓝牙管理工具，后面用到的bluetoothctl命令就是Bluez软件包里面的。

PulseAudio是一个跨平台的、可通过网络工作的声音服务，它可以从一个或多个音源（进程或输入设备）接受声音输入
然后重定向声音到一个或多个槽（声卡，远程网络PulseAudio服务，或其他进程），是目前linux环境下的一个主流高级音频解决方案，与ALSA不同的是，PulseAudio不与底层声卡直接打交道（因为这是ALSA的工作），PulseAudio是介于音频播放软件(如omxplayer,
mplayer等)和ALSA之间，它的功能主要集中在与各种音频播放软件对接、多路音频混合、音频转发（到不同的声卡或者网卡）等，有了它我们就能轻松实现给不同的音频源设置不同的音量，音频流媒体等高级功能。

有了这些东西，我们就可以通过它们来使用蓝牙播放音乐了。

前期准备
--------

使用ssh连接开发板
~~~~~~~~~~~~~~~~~

为了更方便后续的其他操作，我们可以通过ssh登陆开发板，这样子就可以打开多个终端，具体的操作参考：\ https://tutorial.linux.doc.embedfire.com/zh_CN/latest/linux_basis/fire-config_brief.html?highlight=ssh#fire-configssh

插入免驱USB蓝牙
~~~~~~~~~~~~~~~

插入\ **免驱USB蓝牙**\ 到开发板的USB接口，然后运行以下命令，可以看到蓝牙插入了：

.. code:: bash

    dmesg

    [  493.457504] usb 1-1.3: new full-speed USB device number 3 using ci_hdrc

如果在dmesg的最后出现了上述内容，那么表示此时开发板已经识别到USB蓝牙设备了，此时可以接下来的后续操作，如果没能识别，则检查USB蓝牙设备是否为免驱的，如果不是免驱的则需要你自己移植驱动了，此处不过多讲解。

当然也可以用hciconfig命令验证，通过hciconif可以查看蓝牙设备的具体信息，此时可以看到系统已经识别到这个是USB蓝牙设备了：

.. code:: bash

    ➜  ~ hciconfig

    hci0:   Type: Primary  Bus: USB
            BD Address: 00:1A:7D:DA:71:13  ACL MTU: 310:10  SCO MTU: 64:8
            UP RUNNING 
            RX bytes:1220 acl:0 sco:0 events:72 errors:0
            TX bytes:2788 acl:0 sco:0 commands:72 errors:0

更新
~~~~

为了确保我们安装的软件包的版本是最新版本，让我们使用apt命令更新本地apt包索引和升级系统：

.. code:: bash

    sudo apt-get update
    sudo apt-get -y upgrade

手动安装相关的依赖包
~~~~~~~~~~~~~~~~~~~~

这些依赖包是这次测试libmodbus必须的，要安装一下。

-  安装蓝牙协议栈相关的软件包。

.. code:: bash

    sudo apt-get -y install bluez

.. code:: bash

    sudo apt-get -y install blueman

-  安装声音服务相关的软件包pulseaudio。

.. code:: bash

    sudo apt-get -y install pulseaudio

.. code:: bash

    sudo apt install pulseaudio-module-bluetooth 

-  安装bluealsa，它是BlueZ与ALSA直接集成的结果。由于BlueZ版本 >=
   5，已删除内置集成，而支持第三方音频应用程序。从现在开始，BlueZ就充当实现蓝牙音频配置文件的音频应用程序和蓝牙音频设备之间的中间件。使用bluealsa可以实现与PulseAudio相同的目标，但依赖性更少，bluealsa在BlueZ中注册了所有已知的蓝牙音频配置文件，因此理论上每个蓝牙设备（具有音频功能）都可以播放音频。由于Debian软件包并没有bluealsa这个软件包，我们只能从官方发布的软件包中去下载，因此安装wget去获取bluealsa\_0.13\_armhf.deb软件包，然后通过dpkg命令去安装它。

.. code:: bash

    sudo apt-get -y install wget

.. code:: bash

    wget https://archive.raspberrypi.org/debian/pool/main/b/bluealsa/bluealsa_0.13_armhf.deb

    sudo dpkg -i bluealsa_0.13_armhf.deb

-  安装alsa工具。

.. code:: bash

    sudo apt-get -y install alsa-utils

上面的软件包安装完成后，将当前用户添加到蓝牙组：

.. code:: bash

    sudo usermod -G bluetooth -a $USER

重启开发板
~~~~~~~~~~

操作完成后，重启开发板。

连接蓝牙音箱
------------

在重启了开发板后，启动蓝牙功能。

.. code:: bash

    sudo hciconfig hci0 up

在进行音频播放前，需要通过bluetoothctl命令搜索到目标蓝牙音频设备，并与其进行配对连接。进入bluetoothctl命令行操作环境，bluetoothctl是一个非常强大的蓝牙工具，通过它可以搜索到周边的蓝牙设备，并与指定的蓝牙设备建立连接、断开连接，直接运行bluetoothctl即可进入它的命令行操作环境。

.. code:: bash

    ➜  ~ bluetoothctl

    Agent registered
    [bluetooth]# power on
    Changing power on succeeded
    [bluetooth]# agent on
    Agent is already registered
    [bluetooth]# default-agent
    Default agent request successful
    [bluetooth]# scan on
    Discovery started
    [CHG] Controller 00:1A:7D:DA:71:13 Discovering: yes
    [NEW] Device F4:60:E2:6D:FF:AB 小米手机
    [NEW] Device 00:BB:C1:3E:40:F7 TS9100 seri
    [NEW] Device E0:DC:FF:F9:DE:4C mi9pro5g
    [NEW] Device F0:13:C3:AB:A4:F3 HUAWEI AM08
    [bluetooth]# connect F0:13:C3:AB:A4:F3 
    Attempting to connect to F0:13:C3:AB:A4:F3
    [CHG] Device F0:13:C3:AB:A4:F3 Connected: yes
    [CHG] Device F0:13:C3:AB:A4:F3 UUIDs: 00001108-0000-1000-8000-00805f9b34fb
    [CHG] Device F0:13:C3:AB:A4:F3 UUIDs: 0000110b-0000-1000-8000-00805f9b34fb
    [CHG] Device F0:13:C3:AB:A4:F3 UUIDs: 0000110c-0000-1000-8000-00805f9b34fb
    [CHG] Device F0:13:C3:AB:A4:F3 UUIDs: 0000110e-0000-1000-8000-00805f9b34fb
    [CHG] Device F0:13:C3:AB:A4:F3 UUIDs: 0000111e-0000-1000-8000-00805f9b34fb
    [CHG] Device F0:13:C3:AB:A4:F3 ServicesResolved: yes
    [CHG] Device F0:13:C3:AB:A4:F3 Paired: yes
    Connection successful
    [HUAWEI AM08]# quit

从上面的内容（看#号后面的），可以看到我只输入了以下的命令，它们分别对应的功能说明如下：

.. code:: bash

    power on        -- 启动蓝牙模块
    agent on        -- 蓝牙模块agent 打开
    default-agent   -- 设置agent为默认模式
    scan on         -- 扫描周边设备

当我们扫描的时候，它会列出当前可用的蓝牙设备，我们只需要使用connect命令去连接它即可，指定连接蓝牙设备的MAC地址，最后可用听到蓝牙音箱提示连接成功，而在bluetoothctl命令行操作环境也可以看到提示连接成功Connection
successful。

.. code:: bash

    # 发现了蓝牙音箱，MAC地址是 F0:13:C3:AB:A4:F3
    [NEW] Device F0:13:C3:AB:A4:F3 HUAWEI AM08

    connect [MAC]   -- 连接指定的MAC地址设备

当然更多的功能也可以在bluetoothctl命令行操作环境通过help去了解到，此处就不一一列举了。

播放音乐
--------

首先在开发板中放入一个音频文件，我就直接通过wget去下载比较方便一点。

.. code:: bash

    wget -O example.wav https://www.firebbs.cn/forum.php?mod=attachment&aid=MjYzMDF8M2U3MWY1NWF8MTU5MTA4NDMyOHwzNzM5M3wyNjI3NA%3D%3D

可以看到在下载完成后，当前目录下出现了一个example.wav音频测试文件，我们就播放它即可。

播放音乐的命令如下，F0:13:C3:AB:A4:F3则是指定蓝牙设备的MAC地址，也就是我们前面提到的连接蓝牙设备的MAC地址。

.. code:: bash

    aplay -D bluealsa:SRV=org.bluealsa,DEV=F0:13:C3:AB:A4:F3,PROFILE=a2dp example1.wav

此时可以听到蓝牙音箱已经可以播放音乐了，本章的介绍就到此结束。
