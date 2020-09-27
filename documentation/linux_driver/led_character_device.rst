.. vim: syntax=rst


字符设备驱动——点亮LED灯
==============================

通过字符设备章节的学习，我们已经了解了字符设备驱动程序的基本框架，主要是掌握如何申请及释放设备号、
添加以及注销设备，初始化、添加与删除cdev结构体，并通过cdev_init函数建立cdev和file_operations之间的关联，
cdev结构体和file_operations结构体非常重要，希望大家着重掌握。

本小节我们将带领大家做一个激动人心的小实验--点亮led。
前面我们已经通过操作寄存器的方式点亮了LED，本节我们将带领大家进入点亮开发板RGB LED灯的世界，
学习一下如何在linux环境下驱动RGB LED灯。

首先我们需要明白直接操作寄存器和通过驱动程序点亮LED有什么区别。



设备驱动的作用与本质
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
直接操作寄存器点亮LED和通过驱动程序点亮LED最本质的区别就是有无使用操作系统。
有操作系统的存在则大大降低了应用软件与硬件平台的耦合度，它充当了我们硬件与应用软件之间的纽带，
使得应用软件只需要调用驱动程序接口API就可以让硬件去完成要求的开发，而应用软件则不需要关心硬件到底是如何工作的。
这将大大提高我们应用程序的可移植性和开发效率。


驱动的作用
------------------------------
设备驱动与底层硬件直接打交道，按照硬件设备的具体工作方式读写设备寄存器，
完成设备的轮询、中断处理、DMA通信，进行物理内存向虚拟内存的映射，最终使通信设备能够收发数据，
使显示设备能够显示文字和画面，使存储设备能够记录文件和数据。

在系统中没有操作系统的情况下，工程师可以根据硬件设备的特点自行定义接口，如对LED定义LightOn()、LightOff()等。
而在有操作系统的情况下，设备驱动的架构则由相应的操作系统定义，驱动工程师必须按照相应的架构设计设备驱动，
如在本次实验中必须设计file_operations的接口。这样，设备驱动才能良好地整合到操作系统的内核中。


有无操作系统的区别
------------------------------
1）无操作系统（即裸机）时的设备驱动
也就是直接操作寄存器的方式控制硬件，在这样的系统中，虽然不存在操作系统，但是设备驱动是必须存在的。
一般情况下，对每一种设备驱动都会定义为一个软件模块，包含.h文件和.c文件，前者定义该设备驱动的数据结构并声明外部函数，
后者进行设备驱动的具体实现。其他模块需要使用这个设备的时候，只需要包含设备驱动的头文件然后调用其中的外部接口函数即可。
这在STM32的开发中很常见，也相对比较简单。

2）有操作系统时的设备驱动
反观有操作系统时，首先，驱动硬件工作的的部分仍然是必不可少的，其次，我们还需要将设备驱动融入内核。
为了实现这种融合，必须在所有的设备驱动中设计面向操作系统内核的接口，这样的接口由操作系统规定，对一类设备而言结构一致，独立于具体的设备。

由此可见，当系统中存在操作系统的时候，设备驱动变成了连接硬件和内核的桥梁。操作系统的存在势必要求设备驱动附加更多的代码和功能，
把单一的驱动变成了操作系统内与硬件交互的模块，它对外呈现为操作系统的API。

操作系统的存在究竟带来了什么好处呢？

首先操作系统完成了多任务并发；
其次操作系统为我们提供了内存管理机制，32位Linux操作系统可以让每个进程都能独立访问4GB的内存空间；
对于应用程序来说，应用程序将可使用统一的系统调用接口来访问各种设备，
通过write()、read()等函数读写文件就可以访问各种字符设备和块设备，而不用管设备的具体类型和工作方式。

内存管理单元MMU
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
在linux环境直接访问物理内存是很危险的，如果用户不小心修改了内存中的数据，很有可能造成错误甚至系统崩溃。
为了解决这些问题内核便引入了MMU，

MMU的功能
------------------------------
MMU为编程提供了方便统一的内存空间抽象，其实我们的程序中所写的变量地址是虚拟内存当中的地址，
倘若处理器想要访问这个地址的时候，MMU便会将此虚拟地址（Virtual Address）翻译成实际的物理地址（Physical Address），
之后处理器才去操作实际的物理地址。MMU是一个实际的硬件，并不是一个软件程序。他的主要作用是将虚拟地址翻译成真实的物理地址同时管理和保护内存，
不同的进程有各自的虚拟地址空间，某个进程中的程序不能修改另外一个进程所使用的物理地址，以此使得进程之间互不干扰，相互隔离。
而且我们可以使用虚拟地址空间的一段连续的地址去访问物理内存当中零散的大内存缓冲区。很多实时操作系统都可以运行在无MMU的CPU中，
比如uCOS、FreeRTOS、uCLinux，以前想CPU也运行linux系统必须要该CPU具备MMU，但现在Linux也可以在不带MMU的CPU中运行了。
总体而言MMU具有如下功能：

- **保护内存：** MMU给一些指定的内存块设置了读、写以及可执行的权限，这些权限存储在页表当中，MMU会检查CPU当前所处的是特权模式还是用户模式，如果和操作系统所设置的权限匹配则可以访问，如果CPU要访问一段虚拟地址，则将虚拟地址转换成物理地址，否则将产生异常，防止内存被恶意地修改。

- **提供方便统一的内存空间抽象，实现虚拟地址到物理地址的转换：** CPU可以运行在虚拟的内存当中，虚拟内存一般要比实际的物理内存大很多，使得CPU可以运行比较大的应用程序。

到底什么是虚拟地址什么是物理地址？

当没有启用MMU的时候，CPU在读取指令或者访问内存时便会将地址直接输出到芯片的引脚上，此地址直接被内存接收，这段地址称为物理地址，
如下图所示。

.. image:: ./media/MMU02.PNG
   :align: center
   :alt: 找不到图片03|

简单地说，物理地址就是内存单元的绝对地址，好比你电脑上插着一张8G的内存条，则第一个存储单元便是物理地址0x0000，
内存条的第6个存储单元便是0x0005，无论处理器怎样处理，物理地址都是它最终的访问的目标。

当CPU开启了MMU时，CPU发出的地址将被送入到MMU，被送入到MMU的这段地址称为虚拟地址，
之后MMU会根据去访问页表地址寄存器然后去内存中找到页表（假设只有一级页表）的条目，从而翻译出实际的物理地址，
如下图所示。

.. image:: ./media/MMU01.PNG
   :align: center
   :alt: 找不到图片03|

对于I.MX 6ULL 这种32位处理器而言，其虚拟地址空间共有4G(2^32),一旦CPU开启了MMU，
任何时候CPU发出的地址都是虚拟地址，为了实现虚拟地址到物理地址之间的映射，
MMU内部有一个专门存放页表的页表地址寄存器，该寄存器存放着页表的具体位置，
用ioremap映射一段地址意味着使用户空间的一段地址关联到设备内存上，
这使得只要程序在被分配的虚拟地址范围内进行读写操作，实际上就是对设备（寄存器）的访问。 

TLB的作用
------------------------------
讲到MMU我又忍不住和大家说下TLB（Translation Lookaside Buffer）的作用。
由上面的地址转换过程可知，当只有一级页表进行地址转换的时候，CPU每次读写数据都需要访问两次内存，
第一次是访问内存中的页表，第二次是根据页表找到真正需要读写数据的内存地址；
如果使用两级了表，那么CPU每次读写数据都需要访问3次内存，这样岂不是显得非常繁琐且耗费CPU的性能，

那有什么更好的解决办法呢？答案是肯定的，为了解决这个问题，TLB便孕育而生。
在CPU传出一个虚拟地址时，MMU最先访问TLB，假设TLB中包含可以直接转换此虚拟地址的地址描述符，
则会直接使用这个地址描述符检查权限和地址转换，如果TLB中没有这个地址描述符，
MMU才会去访问页表并找到地址描述符之后进行权限检查和地址转换，
然后再将这个描述符填入到TLB中以便下次使用，实际上TLB并不是很大，
那TLB被填满了怎么办呢？如果TLB被填满，则会使用round-robin算法找到一个条目并覆盖此条目。

由于MMU非常复杂，在此我们不做过于深入的了解，大家只要大概知道它的作用即可，
感兴趣的同学可以到网上查阅相关资料，对于初学者，还是建议先掌握全局，然后再深挖其中重要的细节，
千万不能在汪洋大海中迷失了方向。本小结我们主要用到的是MMU的地址转换功能，在linux环境中，
我们开启了MMU之后想要读写具体的寄存器（物理地址），就必须用到物理地址到虚拟地址的转换函数。


地址转换函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
上面提到了物理地址到虚拟地址的转换函数。包括ioremap()地址映射和取消地址映射iounmap()函数。

ioremap函数
------------------------------

.. code-block:: c 
    :caption: 地址映射函数 (内核源码/arch/arc/mm/ioremap.c)
    :linenos:

    void __iomem *ioremap(phys_addr_t paddr, unsigned long size)
    #define ioremap ioremap

函数参数和返回值如下：

**参数：**

- **paddr：** 被映射的IO起始地址（物理地址）；
- **size：** 需要映射的空间大小，以字节为单位；

**返回值：** 一个指向__iomem类型的指针，当映射成功后便返回一段虚拟地址空间的起始地址，我们可以通过访问这段虚拟地址来实现实际物理地址的读写操作。

ioremap函数是依靠__ioremap函数来实现的，只是在__ioremap当中其最后一个要映射的I/O空间和权限有关的标志flag为0。
在使用ioremap函数将物理地址转换成虚拟地址之后，理论上我们便可以直接读写I/O内存，但是为了符合驱动的跨平台以及可移植性，
我们应该使用linux中指定的函数（如：iowrite8()、iowrite16()、iowrite32()、ioread8()、ioread16()、ioread32()等）去读写I/O内存，
而非直接通过映射后的指向虚拟地址的指针进行访问。读写I/O内存的函数如下：

.. code-block:: c 
    :caption: 读写I/O函数
    :linenos:
    
    unsigned int ioread8(void __iomem *addr)
    unsigned int ioread16(void __iomem *addr)
    unsigned int ioread32(void __iomem *addr)
         
    void iowrite8(u8 b, void __iomem *addr)	
    void iowrite16(u16 b, void __iomem *addr)
    void iowrite32(u32 b, void __iomem *addr)

- 第1行：读取一个字节（8bit）
- 第2行：读取一个字（16bit）
- 第3行：读取一个双字（32bit）
- 第5行：写入一个字节（8bit）
- 第6行：写入一个字（16bit）
- 第7行：写入一个双字（32bit）

对于读I/O而言，他们都只有一个__iomem类型指针的参数，指向被映射后的地址，返回值为读取到的数据据；
对于写I/O而言他们都有两个参数，第一个为要写入的数据，第二个参数为要写入的地址，返回值为空。
与这些函数相似的还有writeb、writew、writel、readb、readw、readl等，
在ARM架构下，writex（readx）函数与iowritex（ioreadx）有一些区别，
writex（readx）不进行端序的检查，而iowritex（ioreadx）会进行端序的检查。

说了这么多，大家可能还是不太理解，那么我们来举个栗子，比如我们需要操作RGB灯中的蓝色led中的数据寄存器，
在51或者STM32当中我们是直接看手册查找对应的寄存器，然后往寄存器相应的位写入数据0或1便可以实现LED的亮灭（假设已配置好了输出模式以及上下拉等）。
前面我们在不带linux的环境下也是用的类似的方法，但是当我们在linux环境且开启了MMU之后，
我们就要将LED灯引脚对应的数据寄存器（物理地址）映射到程序的虚拟地址空间当中，然后我们就可以像操作寄存器一样去操作我们的虚拟地址啦！其具体代码如下所示。

.. code-block:: c 
    :caption: 地址映射
    :linenos:
	
	unsigned long pa_dr = 0x20A8000 + 0x00; 
    unsigned int __iomem *va_dr;
    unsigned int val;
    va_dr = ioremap(pa_dr, 4);	
    val = ioread32(va_dr);
    val &= ~(0x01 << 19);
    iowrite32(val, va_dr);

- 第1行：Address: Base address + 0h offset
- 第2行：定义一个__iomem类型的指针
- 第4行：将va_dr指针指向映射后的虚拟地址起始处，这段地址大小为4个字节
- 第5行：读取被映射后虚拟地址的的数据，此地址的数据是实际数据寄存器（物理地址）的数据
- 第7行：将蓝色LED灯引脚对应的位清零
- 第8行：把修改后的值重新写入到被映射后的虚拟地址当中，实际是往寄存器中写入了数据

iounmap函数
------------------------------
iounmap函数定义如下：

.. code-block:: c 
    :caption: 取消地址映射函数 (内核源码/arch/arc/mm/ioremap.c)
    :linenos:

    void iounmap(void *addr)
    #define iounmap iounmap

函数参数和返回值如下：

**参数：**

- **addr：** 需要取消ioremap映射之后的起始地址（虚拟地址）。

**返回值：** 无

例如我们要取消一段被ioremap映射后的地址可以用下面的写法。

.. code-block:: c 
    :caption: 取消ioremap映射地址
	:linenos:

    iounmap(va_dr);	//释放掉ioremap映射之后的起始地址（虚拟地址）


点亮LED灯实验
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
从第一章内核模块再到第二章字符设备驱动，从理论到实验，总算是一切准备就绪，让我们开始着手写LED的驱动代码吧。
首先我们需要一个LED字符设备结构体，它应该包含我们要操作的寄存器地址。
其次是模块的加载卸载函数，加载函数需要注册设备，卸载函数则需要释放申请的资源。
然后就是file_operations结构体以及open，write，read相关接口的实现。

实验说明
------------------------------

硬件介绍
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

本节实验使用到 EBF6ULL-PRO 开发板上的 RGB 彩灯。

硬件原理图分析
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
了解RGB灯的实物后，可打开相应的原理图文档来查看硬件连接，即《野火_EBF6ULL S1 Pro 底板_V1.0_原理图》和
《野火_EBF6ULL S1 邮票孔核心板_V1.0_原理图》，具体见下图。

.. image:: ./media/hd_principle001.png
   :align: center
   :alt: LED引脚

.. image:: ./media/hd_principle002.png
   :align: center
   :alt: i.MX6ULL引脚

.. image:: ./media/hd_principle003.png
   :align: center
   :alt: 引脚复用

LED_R的阴极连接到i.MX6ULL芯片上GPIO1_IO04引脚，LED_G连接到CSI_HSYNC,LED_B连接到CSI_VSYNC。
而CSI_HSYNC和CSI_VSYNC作为摄像头的某一功能被复用为GPIO。如下表所示。

.. csv-table::  
    :header: "LED灯", "原理图的标号","具体引脚名","GPIO端口及引脚编号"
    :widths: 15, 15,15,15

	"R灯",	"GPIO_4",	"GPIO1_IO04",	"GPIO1_IO04"
	"G灯",	"CSI_HSYNC",	"CSI_HSYNC",	"GPIO4_IO20"
	"B灯",	"CSI_VSYNC",	"CSI_VSYNC",	"GPIO4_IO19"

对于RGB灯的控制进行控制，也就是对上述GPIO的寄存器进行读写操作。可大致分为以下几个步骤：

- 使能GPIO时钟
- 设置引脚复用为GPIO
- 设置引脚属性(上下拉、速率、驱动能力)
- 控制GPIO引脚输出高低电平

对RGB的R灯进行寄存器配置
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**GPIO时钟**

跟GPIO相关的时钟主要有CCM_CCGR(0~3)寄存器。查看数据手册可以知道GPIO第26-27两位控制引脚时钟。

.. image:: ./media/hd_principle004.png
   :align: center
   :alt: CCM_CCGR寄存器

.. image:: ./media/hd_principle005.png
   :align: center
   :alt: CCM_CCGR寄存器

两个bit的不同取值，设置GPIO时钟的不同属性如下：

.. image:: ./media/hd_principle006.png
   :align: center
   :alt: CCM_CCGR寄存器配置

- 00：所有模式下都关闭外设时钟
- 01：只有在运行模式下打开外设时钟
- 10：保留
- 11：除了停止模式以外，该外设时钟全程使能

CCM_CCGR1地址为 0x20C406C。
先对CCM_CCGR1寄存器的26位、27位值清空，再赋值为11(位运算，详见示例代码)。

**引脚复用GPIO**

引脚复用相关的寄存器为IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04

.. image:: ./media/hd_principle007.png
   :align: center
   :alt: 引脚复用

关于该寄存器的配置可以见下图：

.. image:: ./media/hd_principle008.png
   :align: center
   :alt: 寄存器配置

寄存器地址为0x20E006C，对该寄存器第0-3位配置为 0101时，MUX_MODE为ALT5 ，也就是该引脚复用为GPIO。

**引脚属性**

寄存器为 IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04

.. image:: ./media/hd_principle009.png
   :align: center
   :alt: 寄存器配置

.. image:: ./media/hd_principle010.png
   :align: center
   :alt: 寄存器配置

- HYS（bit16）：用来使能迟滞比较器 。
- PUS（bit15-bit14）：用来设置上下拉电阻大小。
- PUE（bit13）：当 IO 作为输入的时候，这个位用来设置 IO 使用上下拉还是状态保持器。
- PKE（bit12）：用来使能或者禁止上下拉/状态保持器功能。
- ODE（bit11）：IO 作为输出的时候，此位用来禁止或者使能开漏输出。
- SPEED（bit7-bit6）：当 IO 用作输出的时候，此位用来设置 IO 速度。
- DSE（bit5-bit3）：当 IO 用作输出的时候用来设置 IO 的驱动能力。
- SRE（bit0）：设置压摆率

寄存器地址为0x20E02F8，对该寄存器写入0x1F838，也就是二进制 1 1111 1000 0011 1000，对比上图了解引脚的属性。

**输出电平**

.. image:: ./media/hd_principle011.png
   :align: center
   :alt: 寄存器配置

​	- 0：输入
​	- 1：输出

硬件原理以及寄存器配置到此为止，更多硬件上的信息可以查看原理图和芯片手册。

代码讲解
------------------------------

**本章的示例代码目录为：base_code/linux_driver/led_cdev/**

编写LED字符设备结构体且初始化
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c 
    :caption: led字符设备结构体
    :linenos:

    struct led_chrdev {
    	struct cdev dev;
		unsigned int __iomem *va_dr;
    	unsigned int __iomem *va_gdir;
    	unsigned int __iomem *va_iomuxc_mux;
    	unsigned int __iomem *va_ccm_ccgrx;	
    	unsigned int __iomem *va_iomux_pad;	
	
    	unsigned long pa_dr;
    	unsigned long pa_gdir;
    	unsigned long pa_iomuxc_mux;
    	unsigned long pa_ccm_ccgrx;	
    	unsigned long pa_iomux_pad;	
	
    	unsigned int led_pin;
    	unsigned int clock_offset;
    };

    static struct led_chrdev led_cdev[DEV_CNT] = {
    	{.pa_dr = 0x0209C000,.pa_gdir = 0x0209C004,.pa_iomuxc_mux =
    	0x20E006C,.pa_ccm_ccgrx = 0x20C406C,.pa_iomux_pad =
    	0x20E02F8,.led_pin = 4,.clock_offset = 26},	
    	{.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
    	0x20E01E0,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
    	0x20E046C,.led_pin = 20,.clock_offset = 12},
    	{.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
    	0x20E01DC,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
    	0x20E0468,.led_pin = 19,.clock_offset = 12},
    };

在上面的代码中我们定义了一个RGB灯的结构体，并且定义且初始化了一个RGB灯的结构体数组，
因为我们开发板上面共有3个RGB灯，所以代码中DEV_CNT为3。
在初始化结构体的时候我们以“.”+“变量名字”的形式来访问且初始化结构体变量的，
初始化结构体变量的时候要以“，”隔开，使用这种方式简单明了，方便管理数据结构中的成员。

- 第2行：描述一个字符设备的结构体
- 第3行：数据寄存器虚拟地址指针
- 第4行：输入输出方向寄存器虚拟地址指针
- 第5行：端口复用寄存器虚拟地址指针
- 第6行：时钟寄存器虚拟地址指针
- 第7行：电气属性寄存器虚拟地址指针
- 第9行：装载数据寄存器（物理地址）的变量
- 第10行：装载输出方向寄存器（物理地址）的变量
- 第11行：装载端口复用寄存器（物理地址）的变量
- 第12行：装载时钟寄存器（物理地址）的变量
- 第13行：装载电气属性寄存器（物理地址）的变量
- 第15行：LED的引脚
- 第16行：时钟偏移地址（相对于CCM_CCGRx）
- 第20-22行：初始化红灯结构体成员变量
- 第23-25行：初始化绿灯结构体成员变量
- 第26-28行：初始化蓝灯结构体成员变量


内核RGB模块的加载和卸载函数
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

第一部分为内核RGB模块的加载函数，其主要完成了以下任务：

- 调用alloc_chrdev_region()函数向系统动态申请一个未被占用的设备号，使用alloc_chrdev_region()相比较于register_chrdev_region()的好处在于不必自己费时间去查看那些是未被占用的设备号，避免了设备号重复问题；
- 调用class_create()函数创建一个RGB灯的设备类；
- 分别给三个LED建立其对应的字符设备结构体cdev和led_chrdev_fops的关联，并且初始化字符设备结构体，最后注册并创建设备。

第二部分为内核RGB模块的卸载函数，其主要完成了以下任务：

- 调用device_destroy()函数用于从linux内核系统设备驱动程序模型中移除一个设备，并删除/sys/devices/virtual目录下对应的设备目录及/dev/目录下对应的设备文件；
- 调用cdev_del()函数来释放散列表中的对象以及cdev结构本身；
- 释放被占用的设备号以及删除设备类。

从下面代码中我们可以看出这三个LED都使用的同一个主设备号，只是他们的次设备号有所区别而已。

.. code-block:: c 
    :caption: 内核RGB模块的加载和卸载函数
    :linenos:

    static __init int led_chrdev_init(void)
    {
    	int i = 0;
    	dev_t cur_dev;
    	printk("led chrdev init\n");
    	alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);
    	led_chrdev_class = class_create(THIS_MODULE, "led_chrdev");
    	for (; i < DEV_CNT; i++) {
    		cdev_init(&led_cdev[i].dev, &led_chrdev_fops);
    		led_cdev[i].dev.owner = THIS_MODULE;
    		cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);
    		cdev_add(&led_cdev[i].dev, cur_dev, 1);
    		device_create(led_chrdev_class, NULL, cur_dev, NULL,DEV_NAME "%d", i);
    	}
    	return 0;
    }
    module_init(led_chrdev_init);
    
    static __exit void led_chrdev_exit(void)
    {
    	int i;
    	dev_t cur_dev;
    	printk("led chrdev exit\n");
    	for (i = 0; i < DEV_CNT; i++) {
    		cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);
    		device_destroy(led_chrdev_class, cur_dev);
    		cdev_del(&led_cdev[i].dev);	
    	}
    	unregister_chrdev_region(devno, DEV_CNT);
    	class_destroy(led_chrdev_class);
    }
    module_exit(led_chrdev_exit);

- 第5行：向动态申请一个设备号
- 第6行：创建设备类
- 第8行：绑定led_cdev与led_chrdev_fops
- 第11行：注册设备
- 第15行：创建设备
- 第19行：模块加载
- 第25行：计算出设备号
- 第26行：删除设备
- 第29行：注销设备
- 第30行：释放被占用的设备号
- 第32行：模块卸载


file_operations结构体成员函数的实现
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c 
    :caption: file_operations中open函数的实现
    :linenos:
    
	/* 打开RGB LED设备函数 */
    static int led_chrdev_open(struct inode *inode, struct file *filp)
    {
    	unsigned int val = 0;
    	struct led_chrdev *led_cdev =(struct led_chrdev *)container_of(inode->i_cdev, struct led_chrdev,dev);	
    	filp->private_data = led_cdev;	
    	
    	printk("open\n");
    	/* 实现地址映射 */
    	led_cdev->va_dr = ioremap(led_cdev->pa_dr, 4);	//,数据寄存器映射，将led_cdev->va_dr指针指向映射后的虚拟地址起始处，这段地址大小为4个字节
    	led_cdev->va_gdir = ioremap(led_cdev->pa_gdir, 4);	//方向寄存器映射
    	led_cdev->va_iomuxc_mux = ioremap(led_cdev->pa_iomuxc_mux, 4);	//端口复用功能寄存器映射
    	led_cdev->va_ccm_ccgrx = ioremap(led_cdev->pa_ccm_ccgrx, 4);	//时钟控制寄存器映射
    	led_cdev->va_iomux_pad = ioremap(led_cdev->pa_iomux_pad, 4);	//电气属性配置寄存器映射
    	/* 配置寄存器 */
    	val = ioread32(led_cdev->va_ccm_ccgrx);	//间接读取寄存器中的数据
    	val &= ~(3 << led_cdev->clock_offset);
    	val |= (3 << led_cdev->clock_offset);	//置位对应的时钟位
    	iowrite32(val, led_cdev->va_ccm_ccgrx);	//重新将数据写入寄存器
    	iowrite32(5, led_cdev->va_iomuxc_mux);	//复用位普通I/O口
    	iowrite32(0x1F838, led_cdev->va_iomux_pad);
    	
    	val = ioread32(led_cdev->va_gdir);
    	val &= ~(1 << led_cdev->led_pin);
    	val |= (1 << led_cdev->led_pin);
    	iowrite32(val, led_cdev->va_gdir);	//配置位输出模式
    	
    	val = ioread32(led_cdev->va_dr);
    	val |= (0x01 << led_cdev->led_pin);
    	iowrite32(val, led_cdev->va_dr);	//输出高电平
    	
    	return 0;
    }

- 第4行：通过led_chrdev结构变量中dev成员的地址找到这个结构体变量的首地址 
- 第5行：把文件的私有数据private_data指向设备结构体led_cdev
- 第9-13行：实现地址映射
- 第15-21行：配置寄存器

file_operations中open函数的实现函数很重要，下面我们来详细分析一下该函数具体做了哪些工作。

1、container_of()函数:

.. image:: ./media/container_of001.PNG
   :align: center
   :alt: 找不到图片03|

在Linux驱动编程当中我们会经常和container_of()这个函数打交道，所以特意拿出来和大家分享一下，其实这个函数功能不多，
但是如果单靠自己去阅读内核源代码分析，那可能非常难以理解，编写内核源代码的大牛随便两行代码都会让我们看的云深不知处，
分析内核源代码需要我们有很好的知识积累以及技术沉淀。
下面我简单跟大家讲解一下container_of()函数的大致工作内容，其宏定义实现如下所示：

.. code-block:: c 
    :caption: container_of()函数 （位于../ebf-buster-linux/driver/gpu/drm/mkregtable.c）
    :linenos:

    #define container_of(ptr, type, member) ({                      \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
            (type *)( (char *)__mptr - offsetof(type,member) );})

函数参数和返回值如下：

**参数：**

- **ptr：** 结构体变量中某个成员的地址
- **type：** 结构体类型
- **member：** 该结构体变量的具体名字

**返回值：** 结构体type的首地址

原理其实很简单，就是通过已知类型type的成员member的地址ptr，计算出结构体type的首地址。
type的首地址 = ptr - size ，需要注意的是它们的大小都是以字节为单位计算的，container_of()函数的如下：

- 判断ptr 与 member 是否为同一类型
- 计算size大小，结构体的起始地址 = (type \*)((char \*)ptr - size)  (注：强转为该结构体指针)

通过此函数我们便可以轻松地获取led_chrdev结构体的首地址了。

2、文件私有数据:

一般很多的linux驱动都会将文件的私有数据private_data指向设备结构体，其保存了用户自定义设备结构体的地址。
自定义结构体的地址被保存在private_data后，可以通过读、写等操作通过该私有数据去访问设备结构体中的成员，
这样做体现了linux中面向对象的程序设计思想。

3、通过ioremap()函数实现地址的映射:

其实ioremap()函数我们之前分析过了，在led_chrdev_open()函数的作用都是一样的，
只是分别对LED灯所用到的CCM_CCGRx时钟控制寄存器、端口复用寄存器、电气属性配置寄存器、数据寄存器以及输入输出方向寄存器都做了地址映射，
这样我们便可以通过操作程序中的虚拟地址来间接的控制物理寄存器，我们在驱动程序描述寄存器不利于驱动模块的灵活使用，
后几个章节我们会带领大家通过设备树（设备树插件）的方式去描述寄存器及其相关属性，
在此先埋下伏笔，循序渐进，顺腾摸瓜，使大家能够真正理解并掌握linux驱动的精髓。

4、通过ioread32()和iowrite32()等函数操作寄存器:

和STM32一样，都要开启I/O引脚对应的时钟、设置其端口的复用（在此复用为普通的GPIO口）、电气属性、输入输出方向以及输出的高低电平等等，
一般我们访问某个地址时都是先将该地址的数据读取到一个变量中然后修改该变量，最后再将该变量写入到原来的地址当中。
注意我们在操作这段被映射后的地址空间时应该使用linux提供的I/O访问函数（如：iowrite8()、iowrite16()、iowrite32()、
ioread8()、ioread16()、ioread32()等），这里再强调一遍，即使理论上可以直接操作这段虚拟地址了但是Linux并不建议这么做。

下面我们接着分析一下file_operations中write函数的实现：

.. code-block:: c 
    :caption: file_operations中write函数的实现
    :linenos:

    static ssize_t led_chrdev_write(struct file *filp, const char __user * buf, size_t count, loff_t * ppos)/* 向RGB LED设备写入数据函数 */
    {
    	unsigned long val = 0;
    	unsigned long ret = 0;
    	int tmp = count;
    	kstrtoul_from_user(buf, tmp, 10, &ret);
    	struct led_chrdev *led_cdev = (struct led_chrdev *)filp->private_data;
    	val = ioread32(led_cdev->va_dr);
    	if (ret == 0)
    		val &= ~(0x01 << led_cdev->led_pin);
    	else
    		val |= (0x01 << led_cdev->led_pin);	
    	iowrite32(val, led_cdev->va_dr);
    	*ppos += tmp;
    	return tmp;
    }

- 第6行：将用户空间缓存区复制到内核空间
- 第7行：文件的私有数据地址赋给led_cdev结构体指针
- 第8行：间接读取数据寄存器中的数据
- 第9-13行：将数据重新写入寄存器中,控制LED亮灭


1、kstrtoul_from_user()函数:

再分析该函数之前，我们先分析一下内核中提供的kstrtoul()函数，理解kstrtoul()函数之后再分析kstrtoul_from_user()就信手拈来了。

.. code-block:: c 
    :caption: kstrtoul()函数解析 （内核源码/include/linux/kernel.h）
    :linenos:

    static inline int __must_check kstrtoul(const char *s, unsigned int base, unsigned long *res)
    {
    	/*
    	 * We want to shortcut function call, but
    	 * __builtin_types_compatible_p(unsigned long, unsigned long long) = 0.
    	 */
    	if (sizeof(unsigned long) == sizeof(unsigned long long) &&
    	    __alignof__(unsigned long) == __alignof__(unsigned long long))
    		return kstrtoull(s, base, (unsigned long long *)res);
    	else
    		return _kstrtoul(s, base, res);
    }

该函数的功能是将一个字符串转换成一个无符号长整型的数据。

函数参数和返回值如下：

**参数：** 

- **s：** 字符串的起始地址，该字符串必须以空字符结尾；
- **base：** 转换基数，如果base=0，则函数会自动判断字符串的类型，且按十进制输出，比如“0xa”就会被当做十进制处理（大小写都一样），输出为10。如果是以0开头则会被解析为八进制数，否则将会被解析成小数；
- **res：** 一个指向被转换成功后的结果的地址。

**返回值：** 该函数转换成功后返回0，溢出将返回-ERANGE，解析出错返回-EINVAL。理解完kstrtoul()函数后想必大家已经知道kstrtoul_from_user()函数的大致用法了，

kstrtoul_from_user()函数定义如下：

.. code-block:: c 
    :caption: kstrtoul_from_user()函数 （内核源码/include/linux/kernel.h）
    :linenos:

    int __must_check kstrtoul_from_user(const char __user *s, size_t count, unsigned int base, unsigned long *res);

函数参数和返回值如下：

**参数：** 

- **s：** 字符串的起始地址，该字符串必须以空字符结尾；
- **count：** count为要转换数据的大小；
- **base：** 转换基数，如果base=0，则函数会自动判断字符串的类型，且按十进制输出，比如“0xa”就会被当做十进制处理（大小写都一样），输出为10。如果是以0开头则会被解析为八进制数，否则将会被解析成小数；
- **res：** 一个指向被转换成功后的结果的地址。

**返回值：**

该函数相比kstrtoul()多了一个参数count，因为用户空间是不可以直接访问内核空间的，所以内核提供了kstrtoul_from_user()函数以实现用户缓冲区到内核缓冲区的拷贝，与之相似的还有copy_to_user()，copy_to_user()
完成的是内核空间缓冲区到用户空io间的拷贝。如果你使用的内存类型没那么复杂，便可以选择使用put_user()或者get_user()函数。

最后分析一下file_operations中release函数的实现：

当最后一个打开设备的用户进程执行close()系统调用的时候，内核将调用驱动程序release()函数，
release函数的主要任务是清理未结束的输入输出操作，释放资源，用户自定义排他标志的复位等。
前面我们用ioremap()将物理地址空间映射到了虚拟地址空间，当我们使用完该虚拟地址空间时应该记得使用iounmap()函数将它释放掉。

.. code-block:: c 
    :caption: file_operations中release函数的实现
    :linenos:

    static int led_chrdev_release(struct inode *inode, struct file *filp)
    {
    	struct led_chrdev *led_cdev = (struct led_chrdev *)container_of(inode->i_cdev, struct led_chrdev, dev);	
    	/* 释放ioremap后的虚拟地址空间 */
    	iounmap(led_cdev->va_dr);	//释放数据寄存器虚拟地址
    	iounmap(led_cdev->va_gdir);	//释放输入输出方向寄存器虚拟地址
    	iounmap(led_cdev->va_iomuxc_mux);	//释放I/O复用寄存器虚拟地址
    	iounmap(led_cdev->va_ccm_ccgrx);	//释放时钟控制寄存器虚拟地址
    	iounmap(led_cdev->va_iomux_pad);	//释放端口电气属性寄存器虚拟地址
    	return 0;
    }

- 第3行：将文件的私有数据地址赋给led_cdev结构体指针
- 第5-9行：释放ioremap后的虚拟地址空间 

LED驱动完整代码
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
到这里我们的代码已经分析完成了，下面时本驱动的完整代码（由于前面已经带领大家详细的分析了一遍，
所以我把完整代码的注释给去掉了，希望你能够会想起每个函数的具体作用）。


**led_cdev.c**

.. code-block:: c 
    :caption: 完整代码 （位于../base_code/linux_driver/EmbedCharDev/led_cdev/led_cdev.c）
    :linenos:

    #include <linux/init.h>
    #include <linux/module.h>
    #include <linux/cdev.h>
    #include <linux/fs.h>
    #include <linux/uaccess.h>
    #include <linux/io.h>
    
    #define DEV_NAME            "led_chrdev"
    #define DEV_CNT                 (3)
    
    static dev_t devno;
    struct class *led_chrdev_class;
    
    struct led_chrdev {
    	struct cdev dev;
    	unsigned int __iomem *va_dr;
    	unsigned int __iomem *va_gdir;
    	unsigned int __iomem *va_iomuxc_mux;
    	unsigned int __iomem *va_ccm_ccgrx;
    	unsigned int __iomem *va_iomux_pad;
    
    	unsigned long pa_dr;
    	unsigned long pa_gdir;
    	unsigned long pa_iomuxc_mux;
    	unsigned long pa_ccm_ccgrx;
    	unsigned long pa_iomux_pad;
    
    	unsigned int led_pin;
    	unsigned int clock_offset;
    };
    
    static int led_chrdev_open(struct inode *inode, struct file *filp)
    {

    	unsigned int val = 0;
    	struct led_chrdev *led_cdev =
    	    (struct led_chrdev *)container_of(inode->i_cdev, struct led_chrdev,
    					      dev);
    	filp->private_data =
    	    container_of(inode->i_cdev, struct led_chrdev, dev);
    
    	printk("open\n");
    
    	led_cdev->va_dr = ioremap(led_cdev->pa_dr, 4);			/*  */  
    	led_cdev->va_gdir = ioremap(led_cdev->pa_gdir, 4);
    	led_cdev->va_iomuxc_mux = ioremap(led_cdev->pa_iomuxc_mux, 4);
    	led_cdev->va_ccm_ccgrx = ioremap(led_cdev->pa_ccm_ccgrx, 4);
    	led_cdev->va_iomux_pad = ioremap(led_cdev->pa_iomux_pad, 4);
    
    	val = ioread32(led_cdev->va_ccm_ccgrx);
    	val &= ~(3 << led_cdev->clock_offset);
    	val |= (3 << led_cdev->clock_offset);
    
    	iowrite32(val, led_cdev->va_ccm_ccgrx);
    	iowrite32(5, led_cdev->va_iomuxc_mux);
    	iowrite32(0x1F838, led_cdev->va_iomux_pad);
    
    	val = ioread32(led_cdev->va_gdir);
    	val &= ~(1 << led_cdev->led_pin);
    	val |= (1 << led_cdev->led_pin);

    	iowrite32(val, led_cdev->va_gdir);
    
    	val = ioread32(led_cdev->va_dr);
    	val |= (0x01 << led_cdev->led_pin);
    	iowrite32(val, led_cdev->va_dr);
    
    	return 0;
    }
    
    
    static int led_chrdev_release(struct inode *inode, struct file *filp)
    {
    	struct led_chrdev *led_cdev =
    	    (struct led_chrdev *)container_of(inode->i_cdev, struct led_chrdev,
    					      dev);
    	iounmap(led_cdev->va_dr);
    	iounmap(led_cdev->va_gdir);
    	iounmap(led_cdev->va_iomuxc_mux);
    	iounmap(led_cdev->va_ccm_ccgrx);
    	iounmap(led_cdev->va_iomux_pad);
    	return 0
    }
    
    static ssize_t led_chrdev_write(struct file *filp, const char __user * buf,
    				size_t count, loff_t * ppos)
    {
    	unsigned long val = 0;
    	unsigned long ret = 0;
    
    	int tmp = count;
    
    	kstrtoul_from_user(buf, tmp, 10, &ret);
    	struct led_chrdev *led_cdev = (struct led_chrdev *)filp->private_data;
    
    	val = ioread32(led_cdev->va_dr);
    	if (ret == 0)
    		val &= ~(0x01 << led_cdev->led_pin);
    	else
    		val |= (0x01 << led_cdev->led_pin);
    
    	iowrite32(val, led_cdev->va_dr);
    	*ppos += tmp;
    	return tmp;
    }
    
    static struct file_operations led_chrdev_fops = {
    	.owner = THIS_MODULE,
    	.open = led_chrdev_open,
    	.release = led_chrdev_release,
    	.write = led_chrdev_write,
    };
    
    static struct led_chrdev led_cdev[DEV_CNT] = {
    	{.pa_dr = 0x0209C000,.pa_gdir = 0x0209C004,.pa_iomuxc_mux =
    	 0x20E006C,.pa_ccm_ccgrx = 0x20C406C,.pa_iomux_pad =
    	 0x20E02F8,.led_pin = 4,.clock_offset = 26},
    	{.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
    	 0x20E01E0,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
    	 0x20E046C,.led_pin = 20,.clock_offset = 12},
    	{.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
    	 0x20E01DC,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
    	 0x20E0468,.led_pin = 19,.clock_offset = 12},
    };
    
    static __init int led_chrdev_init(void)
    {
    	int i = 0;
    	dev_t cur_dev;
    	printk("led chrdev init\n");
    
    	alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);
    
    	led_chrdev_class = class_create(THIS_MODULE, "led_chrdev");
    
    	for (; i < DEV_CNT; i++) {
    
    		cdev_init(&led_cdev[i].dev, &led_chrdev_fops);
    
    		led_cdev[i].dev.owner = THIS_MODULE;
    
    		cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);
    
    		cdev_add(&led_cdev[i].dev, cur_dev, 1);
    
    		device_create(led_chrdev_class, NULL, cur_dev, NULL,
    			      DEV_NAME "%d", i);
    	}
    
    	return 0;
    }
    
    module_init(led_chrdev_init);
    
    static __exit void led_chrdev_exit(void)
    {
    	int i;
    	dev_t cur_dev;
    	printk("led chrdev exit\n");
    
    	for (i = 0; i < DEV_CNT; i++) {
    
    		cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);
    
    		device_destroy(led_chrdev_class, cur_dev);
    
    		cdev_del(&led_cdev[i].dev);
    
    	}
    	unregister_chrdev_region(devno, DEV_CNT);
    	class_destroy(led_chrdev_class);
    }
    
    module_exit(led_chrdev_exit);
    
    MODULE_AUTHOR("embedfire");
    MODULE_LICENSE("GPL");


实验准备
------------------------------

LED驱动Makefile
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: makefile
    :caption: LED驱动Makefile
    :linenos:

	KERNEL_DIR=../ebf-buster-linux/build_image/build
	ARCH=arm
	CROSS_COMPILE=arm-linux-gnueabihf-
	export  ARCH  CROSS_COMPILE

	obj-m := led_cdev.o
	out =  led_cdev_test

	all:
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules
		$(CROSS_COMPILE)gcc -o $(out) led_test.c

	.PHONE:clean copy

	clean:
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean	
		rm $(out)

Makefile与前面的相差不大，定义了led_cdev这个内核模组和led_cdev_test应用程序。

编译命令说明
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

在实验目录下输入如下命令来编译驱动模块：

.. code:: bash

    make

编译成功后，实验目录下会生成"led_cdev.ko"的驱动模块文件和"led_cdev_test"的应用程序。

.. image:: ./media/led_cdev001.png
   :align: center
   :alt: 找不到图片04|

程序运行结果
------------------------------
通过scp或者nfs将上面的两个文件拷贝到开发板中，执行下面的命令加载驱动：

安装LED驱动

::

	sudo insmod led_cdev.ko

然后我们可以在/dev/目录下找到 led_chrdev0 led_chrdev1 led_chrdev2 这三个设备，
我们可以通过直接给设备写入1/0来控制LED的亮灭，也可以通过我们的测试程序来控制LED。

::
	
	#红灯亮
	sudo sh -c 'echo 0 >/dev/led_chrdev0' 
	#红灯灭
	sudo sh -c 'echo 1 >/dev/led_chrdev0' 

运行LED测试程序
sudo ./led_cdev_test LED呈现三种光

.. image:: ./media/led_cdev002.png
   :align: center
   :alt: 找不到图片02

.. image:: ./media/led_cdev003.jpg
   :align: center
   :alt: 找不到图片03

.. image:: ./media/led_cdev004.jpg
   :align: center
   :alt: 找不到图片04

这个时候我们再回味一下设备驱动的作用。
当我们开发一款嵌入式产品时，产品的设备硬件发生变动的时候，我们就只需要更改驱动程序以提供相同的API，
而不用去变动应用程序，就能达到同样的效果，这将减少多少开发成本呢。

.. image:: ./media/led_cdev005.jpg
   :align: center
   :alt: 找不到图片05



