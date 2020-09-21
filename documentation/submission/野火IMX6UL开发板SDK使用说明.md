# 野火IMX6UL开发板SDK

** 本小节的文章为网友投稿，仅供参考 **


* 协议：GPLV2
* 版本说明：
* SDK_Release_Data:2019-12-30
* SDK_VERSION:0.1
* SDK_Author:jackeyt
* SDK_site:https://jackeyt.cn
* 运行环境：Ubuntu 18.04 LTS(WSL、VM、真机均可)
* 开发板：野火IMX6UL Pro开发板 
* soc：NXP IMX6UL,ARM Cortex A7
----------

> 功能说明
* 支持一键运行检查开发编译环境开发
* 支持一键配置、编译UBOOT
* 支持一键配置、编译Linux Kernel
* 支持一键配置、制作文件系统：野火官方的各种文件系统、以及Ubuntu16.04、18.04、Debian9、10
* 支持一键配置QT开发环境(NXP官方环境)[未完成]
* 支持一键第三方开源包[未完成]
* 支持一键编译野火官方APP[未完成]
* 支持一键打包SD可烧录的IMG固件包，并支持Linux、windows(etcher等)工具进进行一键烧录

----------
SDK目录详情：
```
APP  
build.sh  
configs  
docs  
filesystems  
kernel  
output  
packages  
QT  
reamde.md  
scripts  
uboot  
VERSION
```
> 目录说明
* APP         =>野火官方的测试APP、脚本等，详情见文件夹下的readme
* build.sh    =>SDK的主要执行文件
* configs     =>配置文件夹，其中deconf.mk用于配置各个
* docs        =>说明、使用文档，原理图等存放
* filesystems =>文件系统：野火官方的各种文件系统、以及Ubuntu、Debian等
* kernel      =>clone来自野火官方的kernel
* output      =>SDK输出文件夹，用于存放编译生成的各种文件等
* packages    =>第三方的开源APP，用于移植至开发板
* QT          =>野火官方的测试APP、脚本等，详情见文件夹下的readme
* reamde.md   =>说明文档readme，本文档
* scripts     =>SDK的各种脚本集合
* uboot       =>clone来自野火官方的uboot
* VERSION     =>本SDK的版本号



----------
如何安装？

```
git clone https://gitee.com/jackeyt/ebf_IMX6_SDK.git
```
或者：
```
git clone https://github.com/jackeyt/ebf_IMX6_SDK.git
```
----------

如何运行？
```
./build.sh
```

----------

> 主菜单效果：
```
    __________  ______   ______  ____  _______     _____ ____  __ __
   / ____/ __ )/ ____/  /  _/  |/  / |/ / ___/    / ___// __ \/ //_/
  / __/ / __  / /_      / // /|_/ /|   / __ \     \__ \/ / / / ,<
 / /___/ /_/ / __/    _/ // /  / //   / /_/ /    ___/ / /_/ / /| |
/_____/_____/_/ _____/___/_/  /_//_/|_\____/____/____/_____/_/ |_|
               /_____/                    /_____/
SDK_Release_Data:2019-11-30

SDK_VERSION:0.1

SDK_Author:jackeyt

SDK_site:https://jackeyt.cn

[0] Building Env check
[1] Building uboot for ebf_imx6ul
[2] Building linux kernel for ebf_imx6ul
[3] Building filesystems for ebf_imx6ul
[4] Building QT for ebf_imx6ul
[5] Building 3th packages for ebf_imx6ul
[6] Building APPs for ebf_imx6ul
[7] Building QT APPs for ebf_imx6ul
[8] Building and Burning image to sdcard
[9] Exiting SDK Building Guide!
please select:

```
----------
> 功能说明

* ### 0.检查编译环境

运行`build.sh`之后，在主菜单上选择`[0] Building Env check`即可开始检查系统编译环境，效果如下：

```
please select: 0
your select is: 0
Building Env check starting...


checking packages...
```

* 注意需要`root`权限,按提示输入密码!

----------
运行成功结果如下：
```
checking CROSS_COMPILE:arm-linux-gnueabihf-gcc
arm-linux-gnueabihf-gcc is /opt/arm-gcc/bin/arm-linux-gnueabihf-gcc
checking packages success! 
```

### 1.UBOOT选项
运行`build.sh`之后，在主菜单上选择`[1] Building uboot for ebf_imx6u`即可开始检查系统编译环境，效果如下：
```
your select is: 1
Building uboot for ebf_imx6ul starting...
[0] uboot_configure
[1] uboot_build
[2] uboot_clean
[3] uboot_distclean
[4] uboot_menuconfig
```
可以看到uboot选项的支持功能有：
* 0.配置
* 1.编译
* 2.clean清除
* 3.distclean清除
* 4.menuconfig菜单选择

#### 1.0 UBOOT选项—配置
支持三种Flash的配置：NAND、EMMC、SD Card
输入`0`回车进入配置选项菜单：
```
your select is: 0
Configuring Uboot...
Select uboot configs:

1:configs for nand
2:configs for emmc
3:configs for SD Card
```

下面以NAND Flash为例：
输入`1`回车即可：
```
please select configs for uboot: 1
configs for nand
....
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- mx6ull_14x14_evk_nand_defconfig
  HOSTCC  scripts/basic/fixdep
  HOSTCC  scripts/kconfig/conf.o
  SHIPPED scripts/kconfig/zconf.tab.c
  SHIPPED scripts/kconfig/zconf.lex.c
  SHIPPED scripts/kconfig/zconf.hash.c
  HOSTCC  scripts/kconfig/zconf.tab.o
  HOSTLD  scripts/kconfig/conf
#
# configuration written to .config
#
Configured mx6ull_14x14_evk_nand_defconfig to uboot success!
```

#### 1.1 UBOOT选项—编译
输入`1`回车即可：
```
Building uboot for ebf_imx6ul starting...
[0] uboot_configure
[1] uboot_build
[2] uboot_clean
[3] uboot_distclean
please select: 1
your select is: 1

Using Confiugre file:mx6ull_14x14_evk_nand_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j8
scripts/kconfig/conf  --silentoldconfig Kconfig
  CHK     include/config.h
  UPD     include/config.h
  GEN     include/autoconf.mk
  GEN     include/autoconf.mk.dep
  CHK     include/config/uboot.release
............
............
  LD      u-boot
  OBJCOPY u-boot-nodtb.bin
  OBJCOPY u-boot.srec
  SYM     u-boot.sym
  COPY    u-boot.bin
  CFGS    board/freescale/mx6ullevk/imximage.cfg.cfgtmp
  MKIMAGE u-boot.imx
====Build uboot ok!====
```

#### 1.2 UBOOT选项—clean
输入`2`回车即可：
```
Building uboot for ebf_imx6ul starting...
[0] uboot_configure
[1] uboot_build
[2] uboot_clean
[3] uboot_distclean
please select: 2
your select is: 2
Cleaning Uboot...

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- clean

====Clean uboot ok!====
```


#### 1.3 UBOOT选项—distclean
输入`3`回车即可：
```
Building uboot for ebf_imx6ul starting...
[0] uboot_configure
[1] uboot_build
[2] uboot_clean
[3] uboot_distclean
please select: 3
your select is: 3
Distleaning Uboot...
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean
  CLEAN   scripts/basic
  CLEAN   scripts/kconfig
  CLEAN   include/config include/generated
  CLEAN   .config .config.old include/autoconf.mk include/autoconf.mk.dep include/config.h
====distclean uboot ok!====
```

#### 1.4 UBOOT选项—menuconfig

进入`menuconfig`菜单配置模式


### 2.Linux Kernel选项
```
please select again: 2
Building kernel for ebf_imx6ul starting...
[0] linux_configure
[1] linux_distclean
[2] linux_clean
[3] linux_build_dts_only
[4] linux_build_modules_only
[5] linux_build_kernel
[6] linux_build_all
[7] linux_kernel_menuconfig
```
#### 2.0 kernel选项—配置
```
your select is: 0
Configuring kernel...
Select kernel configs:

1:configs for LCD 4.3 inch
2:configs for LCD 5.0 inch
3:configs for LCD 7.0 inch
```
支持三种尺寸LCD的配置：4.3、5.0、7.0

#### 2.1 kernel选项—1~7
> 这里不再赘述，看选项即知道

### 3.filesystems选项

> 在主菜单选择`[3] Building filesystems for ebf_imx6ul`即可进行文件系统编译选项：
```
your select is: 3
Building filesystems for ebf_imx6ul starting...
Select which type fs you want to build:

0:readme
1:base_fs
2:debian 9 Stretch
3:debian 10 Buster
4:ebf_rootfs
5:ebf sato
6:qt5 fs
7:ubuntu 16.04 core
8:ubuntu 18.04 core
please select configs for filesystem:
```
> 说明
----------
* 0：针对文件系统编译说明
* 1：野火官方的最简文件系统
* 2：debian9文件系统
* 3：debian10文件系统
* 4：野火官方的文件系统
* 5：野火官方的文件系统
* 6：野火官方的文件系统带qt5
* 7：ubuntu16.04文件系统
* 8：ubuntu18.04文件系统

关于野火官方的文件系统说明，可见：`filesystems/readme.md`

>下面以编译debian9系统为例[1]：

```
....
I: Extracting libsmartcols1...
I: Extracting libuuid1...
I: Extracting mount...
I: Extracting util-linux...
I: Extracting liblzma5...
I: Extracting zlib1g...
download buster success!

```

```
.....
I: Configuring libc-bin...
I: Configuring systemd...
I: Base system installed successfully.
```


> 下面以编译ubuntu16.04系统为例[1]：
```
......
Resolving mirrors.tuna.tsinghua.edu.cn (mirrors.tuna.tsinghua.edu.cn)... 101.6.8.193, 2402:f000:1:408:8100::1
Connecting to mirrors.tuna.tsinghua.edu.cn (mirrors.tuna.tsinghua.edu.cn)|101.6.8.193|:443... connected.
HTTP request sent, awaiting response... 200 OK
Length: 37806000 (36M) [application/octet-stream]
Saving to: ‘ubuntu-base-16.04.6-base-armhf.tar.gz’

ubuntu-base-16.04.6-base-armh 100%[=================================================>]  36.05M   260KB/s    in 83s

2019-12-08 02:16:34 (443 KB/s) - ‘ubuntu-base-16.04.6-base-armhf.tar.gz’ saved [37806000/37806000]

download ubuntu-base-16.04.6-base-armhf.tar.gz success!
```
* ### 4.QT选项[to be done]
* ### 5.配置编译第三方包[to be done]
* ### 6.官方出厂APP编译[to be done]
* ### 7.QT APPs选项[to be done]
* ### 8.SD卡镜像制作&烧写选项
根据配置编译好的uboot、zImage、filesystem等，打包生成一个可用于烧录到SD卡、EMMC、NAND的固件包，目前只支持生成SD卡固件，可以直接使用linux下的dd命令，或者windows下的烧录工具一键烧录到SD卡，并放至开发板上一键启动。
生成目录：SDK/output/images/sd/