.. vim: syntax=rst

使用设备树插件实现RGB灯驱动
-------------------------------

Linux4.4以后引入了动态设备树（Dynamic Device
Tree），我们这里翻译为“设备树插件”。设备树插件可以理解为主设备树的“补丁”它动态的加载到系统中，并被内核识别。例如我们要在系统中增加RGB驱动，那么我们可以针对RGB这个硬件设备写一个设备树插件，然后编译、加载到系统即可，无需从新编译整个设备树。

设备树插件是在设备树基础上增加的内容，我们之前讲解的设备树语法完全适用，甚至我们可以直接将之前编
写的设备树节点复制到设备树插件里。具体使用方法介绍如下。
本章配套源码和工具位于“~/linux_driver/dynamic_device_tree”目录下

设备树插件格式
~~~~~~~~~~~~~~~~~~~

设备树插件拥有相对固定的格式，甚至可以认为它只是把设备节点加了一个“壳”编译后内核能够动态加载它。格式如下，具体节点省略。

.. code-block:: dts
    :caption: 设备树插件基本格式
    :linenos:

    /dts-v1/; ①
    /plugin/; ②
    
     / {
            fragment@0 {
                target-path = "/"; ③
                __overlay__ {
                    /*在此添加要插入的节点*/
    
                };
            };
     };



标号①处用于指定dts的版本。标号②表示允许使用未定义的引用并记录它们，设备树插件中可以引用主设备树中的节点，而这些“引用的节点”对于设备树插件来说就是未定义的，所以设备树插件应该加上“/plugin/”。标号③指定设备树插件的加载位置，默认我们加载到根节点下，既“target-path =
“/”。

我们要插入的设备及节点或者要引用（追加）的设备树节点放在__overlay_\_ {…}；内。下面我们尝试将上一节编写的RGB灯节点使用设备 树的方式添加到系统中。

创建RGB灯的设备树插件
~~~~~~~~~~~~~~~~~~~~~~~~~~

实现方法很简单，直接复制上一小节RGB灯的设备节点到设备树插件模板中，如下所示。

.. code-block:: dts
    :caption: rgb_led 设备树插件
    :linenos:

    /dts-v1/;
    /plugin/;
    
    /*---------------第一部分--------------------*/
    
    #include "imx6ul-pinfunc.h"
    
    #include "dt-bindings/gpio/gpio.h"
    
    / {
       fragment@0 {
           target-path = "/";
           __overlay__ {
               /*-----------------第二部分----------------*/
               rgb_led2{
                   #address-cells = <1>;
                   #size-cells = <1>;
                   compatible = "fire,rgb_led21";
                   /*红灯节点*/
                   ranges;
                   rgb_led_red@0x020C406C{
                       compatible = "fire,rgb_led_red";
                       reg = <0x020C406C 0x00000004
                              0x020E006C 0x00000004
                              0x020E02F8 0x00000004
                              0x0209C000 0x00000004
                              0x0209C004 0x00000004>;
                       status = "okay";
                   };
                   /*绿灯节点*/
                   rgb_led_green@0x020C4074{
                       compatible = "fire,rgb_led_green";
                       reg = <0x020C4074 0x00000004
                              0x020E01E0 0x00000004
                              0x020E046C 0x00000004
                              0x020A8000 0x00000004
                              0x020A8004 0x00000004>;
                       status = "okay";
                   };
                   /*蓝灯节点*/
                   rgb_led_blue@0x020C4074{
                       compatible = "fire,rgb_led_blue";
                       reg = <0x020C4074 0x00000004
                              0x020E01DC 0x00000004
                              0x020E0468 0x00000004
                              0x020A8000 0x00000004
                              0x020A8004 0x00000004>;
                       status = "okay";
                   };
               };
           };
       };
    };

从以上代码可以看出，代码第一部分是RGB灯设备节点使用到的头文件，第二部分是我们复制、粘贴过来的RGB灯设备节点。简单两步，RGB灯的设备树插件已经做好了，下面重点是编译设备树插件并把设备树插件添加到系统。

编译设备树插件
~~~~~~~~~~~~~~~

设备树插件与设备树一样都是使用DTC工具编译，只不过设备树编译为.dtb。而设备树插件需要编译为.dtbo。
我们可以使用DTC编译命令编译生成.dtbo，但是这样比较繁琐、容易出错。
我们提供一个编译工具，帮助完成这些繁琐的工作，实现“一键式”编译。
编译工具位于“~/linux_driver/dynamic_device_tree/ebf-linux-dtoverlays-master”。
将ebf-linux-dtoverlays-master文件夹复制到linux主机的任意文件夹。使用之前要安装编译工具，执行如下命令安装。

.. code-block:: sh
   :linenos:

   sudo apt-get install device-tree-compiler

要编译的设备树插件源文件放在“~/ebf-linux-dtoverlays-master/overlays/ebf”目录下，
然后回到编译工具的根目录“~/ebf-linux-dtoverlays-master/”执行“make”即可。
生成的.dtbo位于“~/ebf-linux-dtoverlays-master/output”目录下。
例如本章的RGB设备树插件为“imx-fire-rgb-led-overlay.dts”将其拷贝到“~/ebf-linux-dtoverlays-master/overlays/ebf”目录下，
编译之后就会在“~/ebf-linux-dtoverlays-master/output”目录下生成同名的.dtbo文件。
得到.dtbo后，下一步就是将其加载到系统中。需要注意的是，如果你在执行“make”后出现下图报错，
可以尝试先卸载device-tree-compiler（卸载命令为：“sudo apt-get autoremove device-tree-compiler”）,
重新安装，然后在“ebf-linux-dtoverlays-master”目录下修改scripts/basic/fixdep文件的权限，
修改权限命令为：“chmod 777 scripts/basic/fixdep”。

.. image:: ./media/dtboerr001.png
   :align: center
   :alt: 02|

将设备树插件加入系统
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

上一小节我们编译生成了.dtbo。.dtbo 可以被动态的加载到系统，
这一小节介绍两种将设备树插件加入系统的方法。

使用echo命令加载
^^^^^^^^^^^^^^^^^^^^^^^^^^

linux内核从4.4开始支持设备号树插件，支持并不代表默认开启。
所以我们使用之前要配置内核开启这个功能。
如果使用的是我们提供的debian镜像（无论哪个版本）都是开启过了，
无需再配置内核并重新编译。假设使用的是debina镜像，下面介绍具体的加载步骤。

首先将.dtbo复制到开发板的“/lib/firmware”目录下，执行如下命令，创建一个文件夹。

命令：

.. code-block:: sh
   :linenos:

   mkdir /sys/kernel/config/device-tree/overlays/<文件夹名>

该命令的作用很明显，只是在“/sys/kernel/config/device-tree/overlays/”目录下创建了一个文件夹，
这个文件夹的名字可以任意定义，最好能反应对应的设备，例如本例中要插入RGB灯的设备树插件，
则文件夹命名为rgb_led。

文件夹创建成功后使用“echo”命令将设备树插件“放入”刚刚创建的文件夹。命令如下。

命令：

.. code-block:: sh
   :linenos:

   echo <.dtbo位置 > >/sys/kernel/config/device-tree/overlays/<刚刚创建的文件夹>/path

例如我们的.dtbo放在文件名为“imx-fire-rgb-led-overlay.dtbo”，
并且刚刚创建的文件夹为rgb_led.则命令应当为“echo /lib/firmware/imx-fire-rgb-led-overlay.dtbo >/sys/kernel/config/device-tree/overlays/rgb_led/path”
（排版原因，命令在同一行imx-fire-rgb-led-overlay.dtbo之间没有空格）。

执行该命令可能会出现一个警告，直接忽略即可。加载过程中如果不出错不会输出提示信息。

和设备树相同，加载成功后就可以在“/proc/device-tree”目录下找到与插入的设备树节点同名的文件夹，
进入该文件夹还可以看到该节点拥有的属性以及它的子节点，如下所示。


.. image:: ./media/dynami002.png
   :align: center
   :alt: 02|


进入rgb_led 目录，如下所示。


.. image:: ./media/dynami003.png
   :align: center
   :alt: 02|


看到这些文件，证明已经加载成功了。

将设备树插件写入uEnv.txt配置文件
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

上一小节我们使用echo命令加载设备树插件到系统，采用这种方式每次重启后都要重新加载设备树插件。
将设备树插件写入uEnv.txt配置文件系统启动过程中自动从uEnv.txt读取要加载的设备树插件，
我们也不用创建文件夹。使用方法介绍如下：

与使用 echo命令加载相同，需要将要加载的.dtbo放入“/lib/firmware”，
然后打开位于“/boot”目录下的uEnv.txt文件，如下所示。

.. image:: ./media/dynami004.png
   :align: center
   :alt: 02|



从上图可以看出在uEnv.txt文件夹下有很多被屏蔽的设备树插件，这些设备树插件是烧写系统时自带的插件，
为避免它们干扰我们的实验，这里把它们全部屏蔽掉。如果要将RGB的设备树插件写入uEnv.txt也很简单，
参照着写即可。书写格式为“dtoverlay=<设备树插件路径>”。修改完成后保存、退出。执行reboot命令重启系统。
正常情况下我们可以在“/proc/device-tree”找与插入的设备节点同名的文件夹。

加载RGB灯驱动
~~~~~~~~~~~~~~~~

驱动程序和应用程序的使用方法与上一章完全相同，可直接使用上一章的驱动和测试应用程序完成实验，
实验现象完全相同。
