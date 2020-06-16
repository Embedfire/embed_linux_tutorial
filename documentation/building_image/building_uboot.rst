uboot
=====

在前面的文章我们简单了解过uboot相关的概念，此处再回顾一下，总的来说uboot的目的是为了正确引导内核的启动，它分为两个阶段，即boot
+
loader，boot阶段启动系统，初始化硬件设备，建立内存空间映射图，将系统的软硬件带到一个合适的状态，loader阶段将操作系统内核文件加载至内存，之后跳转到内核所在地址运行。

那么我们今天来编译uboot，并教大家简单修改uboot的内容，因为在真正的生产环境中，uboot是使用芯片原厂做好的，只是根据需要修改一下uboot，而不会去大改uboot的源码，这样子没有必要也没有意义，因为uboot就是为了引导内核启动的，启动后uboot就没有什么作用了，与其将uboot改得乱七八糟还不如花时间去维护内核的稳定性。

下载uboot源码
-------------

想要编译uboot，首先我们要下载到uboot的源码，野火官方已经在NXP提供的uboot之上做了修改，并且提交到github与gitee上，大家可以从github与gitee上下载uboot的源码:

**github：**

.. code:: bash

    git clone https://github.com/Embedfire/ebf_6ull_uboot.git

**gitee：**

.. code:: bash

    git clone https://gitee.com/Embedfire/ebf_6ull_uboot.git

当然啦，如果你想下载NXP原厂的uboot怎么办呢？很简单，直接去NXP官方网站下载就好了，打开
http://git.freescale.com/git/
网页，可以看到很多nxp官方提供的镜像，有内核镜像也有uboot镜像，我们下载官方提供的uboot镜像，名字为 ``uboot-imx`` ，我们点击 ``uboot-imx.git`` 链接，跳转到下载页面，可以发现该下载页面中有很多uboot相关的信息，如它的分支，标签等，当然我们要注意的是uboot下载界面中的下载信息，我们可以通过 ``git clone`` 命令去拉取 ``uboot-imx`` 源码，他们的地址就是在下载页面信息的最下方：

.. code:: bash

    Clone 
    git://git.freescale.com/imx/uboot-imx.git
    http://git.freescale.com/git/cgit.cgi/imx/uboot-imx.git

.. figure:: media/building_uboot002.png
   :alt: building_uboot

   building_uboot
.. figure:: media/building_uboot003.png
   :alt: building_uboot

   building_uboot

**很重要的提醒，我们建议大家使用野火提供的仓库进行编译，而不需要去修改官方的uboot源码。**

在下载完源码后，可以查看一下野官方在NXP的源码之上做了什么样的修改，其实改动的次数只有4次，非常简单，因此我们在日常使用的时候，对uboot的修改是非常微小的，完全不必去分析uboot源码的内容，当然如果你有兴趣，那当我没说。

野火修改uboot的次数如下，它主要是是支持一些硬件，比如LCD触摸屏、HDMI，然后删除NXP官方的logo，修改uboot的延时、bootargs参数等，所以说绝大部分的内容是不需要用户去修改的。

.. code:: bash

    ➜  ebf_6ull_uboot git:(master) git log

    commit d48487689c49f2c29013374689decbbb5bf1459d (HEAD -> master, origin/master)
    Author: jiejie <1161959934@qq.com>
    Date:   Wed Feb 26 08:39:48 2020 +0000

        fix bootargs include rw

    commit a3f25a41fda5ee76b0279dba79b396ae6968993c
    Author: pengjie <jiejie.128@163.com>
    Date:   Tue Oct 8 02:55:05 2019 +0800

        1.delete logo 2.bootdelay==0
        
        Signed-off-by: pengjie <jiejie.128@163.com>

    commit 444f285430aa186cff2ff490c6e4ac2e06cf1c13
    Author: pengjie <jiejie.128@163.com>
    Date:   Tue Sep 17 22:43:26 2019 +0800

        1.修改设备树,适配个别的HDMI显示器,使其能够正常的显示
        
        Signed-off-by: pengjie <jiejie.128@163.com>

    commit 1e738ade76db905670555a5e32dd63a1c6362382
    Author: pengjie <jiejie.128@163.com>
    Date:   Mon Aug 5 21:31:59 2019 +0800

        add for 4.3 & 5.0 & 7.0 LCD
        
        Signed-off-by: pengjie <jiejie.128@163.com>

编译前的准备
------------

开发环境选择 **ubuntu18.04**

**安装独立编译工具链**

arm-none-eabi-gcc：v6.3.1

可以通过以下命令进行安装：

.. code:: bash

     sudo apt-get install gcc-arm-none-eabi

**测试arm-none-eabi-gcc安装是否成功**

.. code:: bash

    arm-none-eabi-gcc -v

    # 输出
    gcc version 6.3.1 20170620 (15:6.3.1+svn253039-1build1) 

安装一些必要的库，以确保编译能正常通过：

.. code:: bash

    sudo apt-get install lib32ncurses5 lib32tinfo5 libc6-i386

开始编译
--------

**清除编译信息**

.. code:: bash

    make ARCH=arm clean

在编译uboot的时候需要我们自定义配置，而nxp官方会提供一些默认配置，这些配置在uboot
/configs/目录下，如：

-  mx6ull_14x14_evk_defconfig
-  mx6ull_14x14_evk_emmc_defconfig
-  mx6ull_14x14_evk_nand_defconfig

就是nxp官方为imx6ull提供的配置文件，可以编译生成从SD卡启动的uboot、从emmc启动的uboot以及从nand启动的uboot，我们可以根据需求选择不同的配置文件从而编译不同的uboot。

进入uboot源码目录下，如果想要编译从SD卡启动的uboot，则可以运行以下命令选择SD卡版本的uboot配置（3个版本的uboot任选其一即可）：

.. code:: bash

    命令（编译sd卡版本uboot）
    make ARCH=arm mx6ull_14x14_evk_defconfig

而如果想编译从emmc启动的uboot，则运行：

.. code:: bash

    命令（编译emmc卡版本uboot）
    make ARCH=arm mx6ull_14x14_evk_ emmc _defconfig

想编译从nand启动的uboot，则运行：

.. code:: bash

    命令（编译nand卡版本uboot）
    make ARCH=arm mx6ull_14x14_evk_ nand _defconfig

在运行以上命令后，Makefile会根据对应的配置文件的内容将配置更新到当前目录下的
``.config`` 文件中，如果读者感兴趣可以查看一下该文件的内容，接下来的编译则是根据 ``.config`` 文件的配置进行编译的。

开始编译，运行以下命令：

.. code:: bash

    make -j4 ARCH=arm CROSS_COMPILE=arm-none-eabi-

    ···
      LD      u-boot
      OBJCOPY u-boot-nodtb.bin
      OBJCOPY u-boot.srec
      SYM     u-boot.sym
      COPY    u-boot.bin
      CFGS    board/freescale/mx6ullevk/imximage.cfg.cfgtmp
      MKIMAGE u-boot.imx

命令中带有jN
参数，表示最大使用N个线程编译，如j4则表示最大使用4个线程编译，根据自己电脑配置决定即可，当然线程数量越多，编译越快。在输出信息的最下方，可以看到已经生成对应的uboot（名字是u-boot.imx），可以直接烧录到开发板中。

当编译完成后会在当前目录下生成 ``u-boot.imx`` 文件

.. code:: bash

    ebf_6ull_uboot/u-boot.imx

把它与内核、设备树、文件系统烧录到开发板即可。

尝试在uboot中添加自己的修改
---------------------------

有时候官方的uboot总是不能满足我们的需求，我们得学会修改对应的uboot配置，比如开机的logo，比如显示屏配置，HDMI配置等，我们就简单修改一下我们的配置，让他能在我们的屏幕上运行，也把开机logo替换为我们的logo。

首先我们要知道这些配置写在什么地方，然后才能去修改它，比如我们先改开机logo，找到uboot
/tools/目录下的Makefile文件，大约在200行，就有关于开机logo的配置，具体如下：

.. code:: makefile

    # Generic logo
    ifeq ($(LOGO_BMP),)
    LOGO_BMP= $(srctree)/$(src)/logos/denx.bmp

    # Use board logo and fallback to vendor
    ifneq ($(wildcard $(srctree)/$(src)/logos/$(BOARD).bmp),)
    LOGO_BMP= $(srctree)/$(src)/logos/$(BOARD).bmp
    else
    ifneq ($(wildcard $(srctree)/$(src)/logos/$(VENDOR).bmp),)
    LOGO_BMP= $(srctree)/$(src)/logos/$(VENDOR).bmp
    endif
    endif

    endif # !LOGO_BMP

解释一下Makefile文件的描述：

-  使用ifeq
   判断是否指定了LOGO_BMP变量（该变量表示开机logo图片），如果不指定则使用默认logo图片denx.bmp，该图片在logos目录下。

-  然后判断一下是否存在使用开发板名字命名的图片（如 :math:(BOARD)，它是一个变量的引用，表示开发板的名字），如果是则使用(BOARD).bmp。

-  最后判断一下是否存在以供应商名字命名的图片（如$(VENDOR).bmp），那么很显然，nxp官方提供的uboot必然是以它们的名字命名logo，那么uboot就会使用它们的logo图片，我们可以到uboot/tools/logos目录下查看一番，就会发现存在freescale.bmp文件，如图所示。

    注意：开机logo必须是bmp类型的图片，否则可能出现错误。

.. figure:: media/building_uboot004.png
   :alt: building_uboot

   building_uboot

既然要修改logo，我们把自己的开机logo图片放进去替换原本的logo即可，我们的开机logo如图所示（注意：logo图片格式必须为bmp格式）。

.. figure:: media/building_uboot005.png
   :alt: building_uboot

   building_uboot

这些logo在uboot启动时就会被显示在屏幕上，具体的显示logo的函数在uboot
/board/esd/common/目录下的lcd.c文件中，大约在81行左右，感兴趣的读者可以去看看源码，在这里就不深入分析。

接着我们可以修改显示屏，让我们的开发板支持显示屏的其他尺寸，那么此时就要去配置源码中修改了，nxp官方支持的imx6ull开发板相关的配置源码文件在/uboot/board/freescale/mx6ullevk/目录下的mx6ullevk.c文件中，我们简单修改一下displays这个数组，它是记录了与显示屏相关信息的数组，具体修改如下：

.. code:: c

    struct display_info_t const displays[] = {{
            .bus = MX6UL_LCDIF1_BASE_ADDR,
            .addr = 0,
            .pixfmt = 24,
            .detect = NULL,
            .enable = do_enable_parallel_lcd,
            .mode   = {
                    .name           = "TFT43AB",
                    .xres           = 480,
                    .yres           = 272,
                    .pixclock       = 108695,
                    .left_margin    = 8,
                    .right_margin   = 4,
                    .upper_margin   = 2,
                    .lower_margin   = 4,
                    .hsync_len      = 41,
                    .vsync_len      = 10,
                    .sync           = 0,
                    .vmode          = FB_VMODE_NONINTERLACED
                       }
            },

            {
            .bus = MX6UL_LCDIF1_BASE_ADDR,
            .addr = 0,
            .pixfmt = 24,
            .detect = NULL,
            .enable = do_enable_parallel_lcd,
            .mode   = {
                    .name           = "TFT50AB",
                    .xres           = 800,
                    .yres           = 480,
                    .pixclock       = 108695,
                    .left_margin    = 46,
                    .right_margin   = 22,
                    .upper_margin   = 23,
                    .lower_margin   = 22,
                    .hsync_len      = 1,
                    .vsync_len      = 1,
                    .sync           = 0,
                    .vmode          = FB_VMODE_NONINTERLACED
                       }
            },

            {
            .bus = MX6UL_LCDIF1_BASE_ADDR,
            .addr = 0,
            .pixfmt = 24,
            .detect = NULL,
            .enable = do_enable_parallel_lcd,
            .mode   = {
                    .name           = "TFT70AB",
                    .xres           = 800,
                    .yres           = 480,
                    .pixclock       = 108695,
                    .left_margin    = 46,
                    .right_margin   = 22,
                    .upper_margin   = 23,
                    .lower_margin   = 22,
                    .hsync_len      = 1,
                    .vsync_len      = 1,
                    .sync           = 0,
                    .vmode          = FB_VMODE_NONINTERLACED
                       }
            }

    };

这里的配置是支持3个野火显示屏尺寸的，4.3寸、5寸、7寸、HDMI的适配等，不同的屏幕尺寸稍微不一样，具体阅读以上配置即可，此处不深入研究，都是比较简单的语法，主要是配置硬件相关的信息。

又比如我们在开机时不想让uboot有延时，我们都可以去修改：

在 ``ebf_6ull_uboot/include/configs/mx6_common.h`` 文件中将宏定义改为0：

.. code:: c

    #ifndef CONFIG_BOOTDELAY
    #define CONFIG_BOOTDELAY    0
    #endif

在 ``ebf_6ull_uboot/include/configs/mx6ullevk.h`` 文件中设置uboot传给内核的参数，比如设置console、bootargs、bootcmd等：

**nand 版本的参数：**

.. code:: c

    #define CONFIG_EXTRA_ENV_SETTINGS \
        CONFIG_MFG_ENV_SETTINGS \
        "panel=TFT50AB\0" \
        "splashimage=0x82000000\0" \
        "fdt_addr=0x83000000\0" \
        "fdt_high=0xffffffff\0"   \
        "console=ttymxc0\0" \
        "bootargs=console=ttymxc0,115200 ubi.mtd=3 "  \
            "root=ubi0:rootfs rw rootfstype=ubifs "          \
            CONFIG_BOOTARGS_CMA_SIZE \
            "mtdparts=gpmi-nand:64m(boot),16m(kernel),16m(dtb),-(rootfs)\0"\
        "bootcmd=nand read ${loadaddr} 0x4000000 0x800000;"\
            "nand read ${fdt_addr} 0x5000000 0x100000;"\
            "bootz ${loadaddr} - ${fdt_addr}\0"

**emmc版本的参数太长了，就不贴代码了，感兴趣的可以自己看源码。**
