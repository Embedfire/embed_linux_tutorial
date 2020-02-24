使用4G模块
==========

野火4G模块
----------

野火EC20 4G模块是由EC20
Pcie接口模组加一个USB转接板构成，可方便插接到带有USB接口的Linux主板上。EC20型号众多，我们选用的型号是CEHCLG，全网通，7模，纯数据、单天线版本。

.. figure:: media/EC20_4G_module001.png
   :alt: media/EC20\_4G\_module001.png

   media/EC20\_4G\_module001.png
EC20是移远的一款4G模组，有Mini
PCle和LCC两种封装，与处理器通信的协议为USB。即你们在市面上看到的那些通过板载的PCle接口与EC20连接的开发板，他们通信时也是通过USB通信，并不是一些初学用户说的通过PCle通信，PCle只是一种封装而已，最终通信都是通过USB。

.. figure:: media/EC20_4G_module002.png
   :alt: media/EC20\_4G\_module002.png

   media/EC20\_4G\_module002.png
.. figure:: media/EC20_4G_module003.png
   :alt: media/EC20\_4G\_module003.png

   media/EC20\_4G\_module003.png
野火开发板使用4G模块
--------------------

野火开发板出厂固件已经支持4G模块的，所以在收到开发板后可以直接使用，使用方式也非常简单，首先插入4G的电话卡到SIM卡座上，这张电话卡必须是可以上网的，然后接上IPX天线（4G模块在发货的时候就已经接好天线的了），最终连接到开发板上，Pro或者mini开发板都可以使用野火4G模块。

.. figure:: media/EC20_4G_module004.png
   :alt: media/EC20\_4G\_module004.png

   media/EC20\_4G\_module004.png
.. figure:: media/EC20_4G_module005.png
   :alt: media/EC20\_4G\_module005.png

   media/EC20\_4G\_module005.png
等待大约6S，可以看到4G模块的蓝色LED灯在闪烁，在开发板串口终端中能看到以下内容，就代表模块启动成功：

.. code:: bash

    root@imx6ull14x14evk:~# usb 1-1.1: new high-speed USB device number 3 using ci_hdrc
    usbcore: registered new interface driver usbserial
    usbcore: registered new interface driver usbserial_generic
    usbserial: USB Serial support registered for generic
    usbcore: registered new interface driver option
    usbserial: USB Serial support registered for GSM modem (1-port)
    option 1-1.1:1.0: GSM modem (1-port) converter detected
    usb 1-1.1: GSM modem (1-port) converter now attached to ttyUSB0
    option 1-1.1:1.1: GSM modem (1-port) converter detected
    usb 1-1.1: GSM modem (1-port) converter now attached to ttyUSB1
    option 1-1.1:1.2: GSM modem (1-port) converter detected
    usb 1-1.1: GSM modem (1-port) converter now attached to ttyUSB2
    option 1-1.1:1.3: GSM modem (1-port) converter detected
    usb 1-1.1: GSM modem (1-port) converter now attached to ttyUSB3
    option 1-1.1:1.4: GSM modem (1-port) converter detected
    usb 1-1.1: GSM modem (1-port) converter now attached to ttyUSB4

拨号上网
--------

模块想要上网则需要拨号，进入 ``~/peripheral/ec20-4g``
目录下，可以看到该目录存在以下文件：

-  ec20\_options
-  ec20\_ppp\_dialer
-  ppp-on.sh

ppp-on.sh就是拨号脚本，直接运行它可以进行拨号，ec20\_options是拨号的配置文件，而ec20\_ppp\_dialer
则是拨号的一些指令。

三个文件的内容如下：

**ec20\_options：**

.. code:: bash

    /dev/ttyUSB2
    115200
    crtscts
    modem
    persist
    lock
    noauth
    noipdefault
    debug
    nodetach
    user Anyname
    password Anypassword
    ipcp-accept-local
    ipcp-accept-remote
    #replacedefaultroute
    defaultroute
    usepeerdns
    noccp
    nobsdcomp
    novj
    #Dump

**ec20\_ppp\_dialer：**

.. code:: bash

    ABORT "NO CARRIER"
    ABORT "ERROR"
    TIMEOUT 120
    "" ATE
    SAY "ATE"
    ECHO ON
    OK ATH
    OK ATP
    OK AT+CGDCONT=1,\"IP\",\"CMNET\"
    #OK AT+ZSNT=0,0,0
    OK ATD*98*1#
    CONNECT

**ppp-on.sh：**

.. code:: bash

    #!/bin/sh
    #clear
    #ppp-on.sh
    OPTION_FILE="ec20_options"
    DIALER_SCRIPT="ec20_ppp_dialer"
    pppd file $OPTION_FILE connect '/usr/sbin/chat -v -f ec20_ppp_dialer' &

那么我们直接运行ppp-on.sh即可：

.. code:: bash

    # 命令
    root@imx6ull14x14evk:~/peripheral/ec20-4g# ./ppp-on.sh

    # 输出
    ATEt@imx6ull14x14evk:~/peripheral/ec20-4g# ATE
    OK
    ATH
    OK
    ATP
    OK
    AT+CGDCONT=1,"IP","CMNET"
    OK
    ATD*98*1#
    CONNECT
    Script /usr/sbin/chat -v -f ec20_ppp_dialer finished (pid 754), status = 0x0
    Serial connection established.
    using channel 1
    Using interface ppp0
    Connect: ppp0 <--> /dev/ttyUSB2
    sent [LCP ConfReq id=0x1 <asyncmap 0x0> <magic 0x77050f7b> <pcomp> <accomp>]
    rcvd [LCP ConfReq id=0x0 <asyncmap 0x0> <auth chap MD5> <magic 0xb48fbdc5> <pcomp> <accomp>]
    sent [LCP ConfAck id=0x0 <asyncmap 0x0> <auth chap MD5> <magic 0xb48fbdc5> <pcomp> <accomp>]
    rcvd [LCP ConfAck id=0x1 <asyncmap 0x0> <magic 0x77050f7b> <pcomp> <accomp>]
    rcvd [LCP DiscReq id=0x1 magic=0xb48fbdc5]
    rcvd [CHAP Challenge id=0x1 <bd2380f085599bb4ff084ce4fe447ad3>, name = "UMTS_CHAP_SRVR"]
    sent [CHAP Response id=0x1 <b30688e3ca3083e649c7a67941fa80b7>, name = "Anyname"]
    rcvd [CHAP Success id=0x1 ""]
    CHAP authentication succeeded
    CHAP authentication succeeded
    sent [IPCP ConfReq id=0x1 <addr 0.0.0.0> <ms-dns1 0.0.0.0> <ms-dns2 0.0.0.0>]
    rcvd [IPCP ConfReq id=0x0]
    sent [IPCP ConfNak id=0x0 <addr 0.0.0.0>]
    rcvd [IPCP ConfNak id=0x1 <addr 10.19.175.34> <ms-dns1 116.116.116.116> <ms-dns2 221.5.88.88>]
    sent [IPCP ConfReq id=0x2 <addr 10.19.175.34> <ms-dns1 116.116.116.116> <ms-dns2 221.5.88.88>]
    rcvd [IPCP ConfReq id=0x1]
    sent [IPCP ConfAck id=0x1]
    rcvd [IPCP ConfAck id=0x2 <addr 10.19.175.34> <ms-dns1 116.116.116.116> <ms-dns2 221.5.88.88>]
    Could not determine remote IP address: defaulting to 10.64.64.64
    local  IP address 10.19.175.34
    remote IP address 10.64.64.64
    primary   DNS address 116.116.116.116
    secondary DNS address 221.5.88.88
    Script /etc/ppp/ip-up started (pid 762)
    Script /etc/ppp/ip-up finished (pid 762), status = 0x0

当你看到拨号返回的内容后有IP地址时，并且状态status =
0x0，就证明拨号成功：

.. code:: bash

    local  IP address 10.19.175.34
    remote IP address 10.64.64.64
    primary   DNS address 116.116.116.116
    secondary DNS address 221.5.88.88
    Script /etc/ppp/ip-up started (pid 762)
    Script /etc/ppp/ip-up finished (pid 762), status = 0x0

验证4G网卡
----------

一旦拨号成功，可以使用ifconfig命令验证4G网卡是否存在，如果成功此时会多出来一个ppp0的网卡，并且已经获取到IP地址了。可以很明显看到，我们目前有多个网卡，eth1，eth2是没有接网线，因此是未连接上的，而ppp0则存在IP地址，那么说明我们可以使用ppp0进行网络通信：

.. code:: bash

    root@imx6ull14x14evk:~/peripheral/ec20-4g# ifconfig

    eth1      Link encap:Ethernet  HWaddr 96:C5:0E:88:93:80
              UP BROADCAST MULTICAST  MTU:1500  Metric:1
              RX packets:0 errors:0 dropped:0 overruns:0 frame:0
              TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
              collisions:0 txqueuelen:1000
              RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

    eth2      Link encap:Ethernet  HWaddr C2:35:09:E7:47:C2
              UP BROADCAST MULTICAST  MTU:1500  Metric:1
              RX packets:0 errors:0 dropped:0 overruns:0 frame:0
              TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
              collisions:0 txqueuelen:1000
              RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

    lo        Link encap:Local Loopback
              inet addr:127.0.0.1  Mask:255.0.0.0
              inet6 addr: ::1%1996162768/128 Scope:Host
              UP LOOPBACK RUNNING  MTU:65536  Metric:1
              RX packets:0 errors:0 dropped:0 overruns:0 frame:0
              TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
              collisions:0 txqueuelen:0
              RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

    ppp0      Link encap:Point-to-Point Protocol
              inet addr:10.17.186.69  P-t-P:10.64.64.64  Mask:255.255.255.255
              UP POINTOPOINT RUNNING NOARP MULTICAST  MTU:1500  Metric:1
              RX packets:4 errors:0 dropped:0 overruns:0 frame:0
              TX packets:4 errors:0 dropped:0 overruns:0 carrier:0
              collisions:0 txqueuelen:3
              RX bytes:52 (52.0 B)  TX bytes:58 (58.0 B)

然后我们可以验证一下ping功能，是否可以访问互联网（此处以ping
baidu.com为示例）：

.. code:: bash

    root@imx6ull14x14evk:~/peripheral/ec20-4g# ping baidu.com

    PING baidu.com (39.156.69.79): 56 data bytes
    64 bytes from 39.156.69.79: seq=0 ttl=48 time=60.056 ms
    64 bytes from 39.156.69.79: seq=1 ttl=48 time=62.634 ms
    64 bytes from 39.156.69.79: seq=2 ttl=48 time=50.397 ms

可能你是接了网线的（假设使用了eth1接口），而默认路由表却不是4G模块，如果想使用网卡上网，则需要更新路由表，解决办法如下：

.. code:: bash

    route del-net 0.0.0.0 eth1
    route add-net 0.0.0.0 ppp0

网速测试
--------

本次测试使用阿里云服务器4M带宽主机，经测试，4G模块已经将4M带宽跑满了，但是还未到模块上限，由于测试主机带宽有限，暂不能做更高速度的测试，但足以证明本模块的性能。

测试结果：

**接收模式：**

.. figure:: media/EC20_4G_module006.png
   :alt: media/EC20\_4G\_module006.png

   media/EC20\_4G\_module006.png
**发送模式：**

.. figure:: media/EC20_4G_module007.png
   :alt: media/EC20\_4G\_module007.png

   media/EC20\_4G\_module007.png

可能出现的问题
--------------

经过实测，目前的脚本是可以连接到移动、联通、电信的，如果出现一些其他的问题，可以参考以下内容：

APN设置:

.. code:: bash

    移动：at+cgdcont=1，"ip"，"cmnet"
    联通：at+cgdcont=1，"ip"，"3gnet"
    电信：at+cgdcont=1，"ip"，"ctnet"

拨号：

.. code:: bash

    移动：*99***1#或*98*1#
    联通：*99#电信：#777

驱动移植
--------

首先拉取\ https://github.com/Embedfire/ebf_6ull_linux\ 仓库，然后进入\ ``ebf_6ull_linux/drivers/usb/serial``\ 目录下，在option.c文件中（大约在626行）添加USB
4G模块的设备ID号和厂家ID号\ ``{ USB_DEVICE(0x2c7c, 0x0125) }``\ ：

.. code:: c

    static const struct usb_device_id option_ids[] = {
        { USB_DEVICE(0x2c7c, 0x0125) },
        { USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COLT) },      
        { USB_DEVICE(OPTION_VENDOR_ID, OPTION_PRODUCT_COLT) },

        ...

    }

    ps：此处仅是示例，源码中已经支持4G模块了，不需要自己添加。

编译
----

**安装必要的库**

.. code:: bash

    sudo apt-get install lzop libncurses5-dev

**安装工具链**

从百度云盘下载\ ``arm-linux-gnueabihf-gcc``\ 编译器的压缩包，版本是
``v4.9.3``

链接：\ https://github.com/Embedfire/products/wiki

在 **Linux系列产品**
中找到的网盘链接，在\ ``i.MX6ULL系列\5-编译工具链\arm-gcc`` 目录下找到
``arm-gcc.tar.gz`` 压缩包并且下载

安装方法参考：\ https://blog.csdn.net/u013485792/article/details/50958253

    **注意**\ ，此处不要用高版本的编译工具链，因为作者亲测新版本的编译器并不能完全兼容，比如新版本编译的内核镜像无法识别到4G模块。

直接运行以下命令进行编译：

::

    ./build.sh

或者...

::

    ./build.sh 5.0

生成的内核镜像与设备树均被拷贝到 ``image`` 目录下。
内核模块相关均被安装到 ``my_lib/lib/``
目录下的\ ``modules``\ 文件夹下，可以直接替换掉\ ``rootfs(根文件系统)``\ 中的\ ``/lib/modules/``\ 。

``build.sh``\ 脚本默认编译5.0寸屏幕的内核镜像，如果需要4.3寸屏幕的内核镜像，则可以使用以下命令去编译:

::

    ./build.sh 4.3

如果你想自己编译，那么可以按照以下步骤编译：

**导出环境变量**

.. code:: bash

    export PATH=/opt/arm-gcc/bin:$PATH
    export ARCH=arm 
    export CROSS_COMPILE=arm-linux-gnueabihf- 

**清除编译信息**

.. code:: bash

    make ARCH=arm clean

设置配置选项,使用野火开发板配置

.. code:: bash

    make ARCH=arm imx6_v7_ebf_defconfig

**开始编译**

.. code:: bash

    make ARCH=arm -j10 CROSS_COMPILE=arm-linux-gnueabihf- 

编译生成的镜像输出路径：

**内核镜像路径**

.. code:: bash

    ebf_6ull_linux/arch/arm/boot

**设备树输出路径**

.. code:: bash

    ebf_6ull_linux/arch/arm/boot/dts

**拷贝zImage与dtb**

可以直接运行脚本\ ``copy.sh``\ 将内核镜像与设备树拷贝到\ ``image``\ 目录下

.. code:: bash

    ./copy.sh

**将内核模块安装到\ ``my_lib``\ 目录下：**

直接替换掉\ ``rootfs(根文件系统)``\ 中的\ ``/lib/modules/``\ 即可。

.. code:: bash

    make -j10;make modules_install ARCH=arm INSTALL_MOD_PATH=my_lib/

关于内核的配置：

**PPP点对点拨号：**

所有PPP相关的都选中

.. code:: bash

     Prompt: PPP (point-to-point protocol) support              
      Location:          
      -> Device Drivers 
        (1)   -> Network device support (NETDEVICES [=y])       

