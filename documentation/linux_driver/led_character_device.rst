.. vim: syntax=rst

字符设备驱动——点亮LED灯
------------------------------------

通过上一小节的学习，我们已经了解了字符设备驱动程序的基本框架，主要是掌握如何申请及释放设备号、添加以及注销设备，
初始化、添加与删除cdev结构体，并通过cdev_init函数建立cdev和file_operations之间的关联，
cdev结构体和file_operations结构体非常重要，希望大家着重掌握，本小节我们将带领大家做一个激动人心的小实验，
点亮led，相信大家和我一样，在人生第一次点亮led的时候内心都满怀激动甚至狂欢，成就感简直爆棚。

本节我们将带领大家进入点亮开发板RGB LED灯的世界，学习一下如何在linux环境下驱动RGB LED灯。

内存管理单元MMU
~~~~~~~~~

MMU的功能
^^^^^^^^^^^

前面我们已经学习了通过汇编或者C的方式直接操作寄存器，从而达到控制LED的目的，那本小结与之前的有什么区别么？
当然有很大的区别，在linux环境直接访问物理内存是很危险的，如果用户不小心修改了内存中的数据，很有可能造成错误甚至系统崩溃。
为了解决这些问题内核便引入了MMU，MMU为编程提供了方便统一的内存空间抽象，其实我们的程序中所写的变量地址是虚拟内存当中的地址，
倘若处理器想要访问这个地址的时候，MMU便会将此虚拟地址（Virtual Address）翻译成实际的物理地址（Physical Address），
之后处理器才去操作实际的物理地址。MMU是一个实际的硬件，并不是一个软件程序。他的主要作用是将虚拟地址翻译成真实的物理地址同时管理和保护内存，
不同的进程有各自的虚拟地址空间，某个进程中的程序不能修改另外一个进程所使用的物理地址，以此使得进程之间互不干扰，相互隔离。
而且我们可以使用虚拟地址空间的一段连续的地址去访问物理内存当中零散的大内存缓冲区。很多实时操作系统都可以运行在无MMU的CPU中，
比如uCOS、FreeRTOS、uCLinux，以前想CPU也运行linux系统必须要该CPU具备MMU，但现在Linux也可以在不带MMU的CPU中运行了。
总体而言MMU具有如下功能：

- 保护内存。MMU给一些指定的内存块设置了读、写以及可执行的权限，这些权限存储在页表当中，
MMU会检查CPU当前所处的是特权模式还是用户模式，如果和操作系统所设置的权限匹配则可以访问，
如果CPU要访问一段虚拟地址，则将虚拟地址转换成物理地址，否则将产生异常，防止内存被恶意地修改。
- 提供方便统一的内存空间抽象，实现虚拟地址到物理地址的转换。CPU可以运行在虚拟的内存当中，
虚拟内存一般要比实际的物理内存大很多，使得CPU可以运行比较大的应用程序。

那么，小朋友！你是否有很多问号？到底什么是虚拟地址什么是物理地址？当没有启用MMU的时候，
CPU在读取指令或者访问内存时便会将地址直接输出到芯片的引脚上，此地址直接被内存接收，
这段地址称为物理地址，如下图所示。

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
任何时候CPU发出的地址都是虚拟地址，为了实现虚拟地址到物理地址之间的映射，MMU内部有一个专门存放页表的页表地址寄存器，
该寄存器存放着页表的具体位置，用ioremap映射一段地址意味着使用户空间的一段地址关联到设备内存上，
这使得只要程序在被分配的虚拟地址范围内进行读写操作，实际上就是对设备（寄存器）的访问。 
由于MMU非常复杂，在此我们不做过于深入的了解，大家只要大概知道它的作用即可，
感兴趣的同学可以到网上查阅相关资料，对于初学者，还是建议先掌握全局，然后再深挖其中重要的细节，
千万不能在汪洋大海中迷失了方向。本小结我们主要用到的是MMU的地址转换功能，在linux环境中，
我们开启了MMU之后想要读写具体的寄存器（物理地址），就必须用到物理地址到虚拟地址的转换函数。

TLB的作用
^^^^^^^^^^^
讲到MMU我又忍不住和大家说下TLB（Translation Lookaside Buffer）的作用，希望大家能够多学点知识，
技多不压身。由上面的地址转换过程可知，当只有一级页表进行地址转换的时候，CPU每次读写数据都需要访问两次内存，
第一次是访问内存中的页表，第二次是根据页表找到真正需要读写数据的内存地址；如果使用两级了表，
那么CPU每次读写数据都需要访问3此内存，这样岂不是显得非常繁琐且耗费CPU的性能，
那有什么更好的解决办法呢？答案是肯定的，为了解决这个问题，TLB便孕育而生。在CPU传出一个虚拟地址时，
MMU最先访问TLB，假设TLB中包含可以直接转换此虚拟地址的地址描述符，则会直接使用这个地址描述符检查权限和地址转换，
如果TLB中没有这个地址描述符，MMU才会去访问页表并找到地址描述符之后进行权限检查和地址转换，
然后再将这个描述符填入到TLB中以便下次使用，实际上TLB并不是很大，
那TLB被填满了怎么办呢？如果TLB被填满，则会使用round-robin算法找到一个条目并覆盖此条目。


地址转换函数
~~~~~~~~~

ioremap函数
^^^^^^^^^^^
ioremap函数在arch/arm/include/asm/io.h（linux4.19）中定义如下：


.. code-block:: c 
    :linenos:

    void __iomem *ioremap(resource_size_t res_cookie, size_t size);
    #define ioremap ioremap

ioremap函数有两个参数：res_cookie、size 和 一个__iomem类型指针的返回值。

- res_cookie:被映射的IO起始地址（物理地址）；
- size:需要映射的空间大小，以字节为单位；
- （__iomem *）：一个指向__iomem类型的指针，当映射成功后便返回一段虚拟地址空间的起始地址，我们可以通过访问这段虚拟地址来实现实际物理地址的读写操作。

ioremap函数是依靠__ioremap函数来实现的，只是在__ioremap当中其最后一个要映射的I/O空间和权限有关的标志flag为0。
在使用ioremap函数将物理地址转换成虚拟地址之后，理论上我们便可以直接读写I/O内存，但是为了符合驱动的跨平台以及可移植性，
我们应该使用linux中指定的函数（如：iowrite8()、iowrite16()、iowrite32()、ioread8()、ioread16()、ioread32()等）去读写I/O内存，
而非直接通过映射后的指向虚拟地址的指针进行访问。读写I/O内存的函数如下：

.. code-block:: c 
    :caption: 读写I/O函数
    :linenos:
    
    unsigned int ioread8(void __iomem *addr)	//读取一个字节（8bit）
    unsigned int ioread16(void __iomem *addr)	//读取一个字（16bit）
    unsigned int ioread32(void __iomem *addr)	//读取一个双字（32bit）
         
    void iowrite8(u8 b, void __iomem *addr)		//写入一个字节（8bit）
    void iowrite16(u16 b, void __iomem *addr)	//写入一个字（16bit）
    void iowrite32(u32 b, void __iomem *addr)	//写入一个双字（32bit）


对于读I/O而言，他们都只有一个__iomem类型指针的参数，指向被映射后的地址，返回值为读取到的数据据；
对于写I/O而言他们都有两个参数，第一个为要写入的数据，第二个参数为
要写入的地址，返回值为空。与这些函数相似的还有writeb、writew、writel、readb、readw、readl等，
在ARM架构下，writex（readx）函数与iowritex（ioreadx）有一些区别，
writex（readx）不进行端序的检查，而iowritex（ioreadx）会进行端序的检查。

说了社么多，大家可能还是不太理解，那么我们来举个栗子，比如我们需要操作RGB灯中的蓝色led中的数据寄存器，
在51或者STM32当中我们是直接看手册查找对应的寄存器，然后往寄存器相应的位写入数据0或1便可以实现LED的亮灭（假设已配置好了输出模式以及上下拉等）。
前面我们在不带linux的环境下也是用的类似的方法，但是当我们在linux环境且开启了MMU之后，
我们就要将LED灯引脚对应的数据寄存器（物理地址）映射到程序的虚拟地址空间当中，
然后我们就可以像操作寄存器一样去操作我们的虚拟地址啦！其具体代码如下所示。

.. code-block:: c 
    :linenos:

    unsigned long pa_dr = 0x20A8000 + 0x00; //Address: Base address + 0h offset
    unsigned int __iomem *va_dr;	//定义一个__iomem类型的指针
    unsigned int val;
    
    va_dr = ioremap(pa_dr, 4);		//将va_dr指针指向映射后的虚拟地址起始处，这段地址大小为4个字节
    
    val = ioread32(va_dr);		//读取被映射后虚拟地址的的数据，此地址的数据是实际数据寄存器（物理地址）的数据
    val &= ~(0x01 << 19);		//将蓝色LED灯引脚对应的位清零
    iowrite32(val, va_dr);		//把修改后的值重新写入到被映射后的虚拟地址当中，实际是往寄存器中写入了数据

iounmap函数
^^^^^^^^^^^
iounmap函数定义如下：


.. code-block:: c 
    :linenos:

    void iounmap(volatile void __iomem *iomem_cookie);
    #define iounmap iounmap

iounmap函数只有一个参数iomem_cookie，用于取消ioremap所映射的地址映射。

- iomem_cookie:需要取消ioremap映射之后的起始地址（虚拟地址）。

例如我们要取消一段被ioremap映射后的地址可以用下面的写法。

.. code-block:: c 
    :linenos:

    iounmap(va_dr);				//释放掉ioremap映射之后的起始地址（虚拟地址）


编写驱动程序
~~~~~~~~~

编写LED字符设备结构体且初始化
^^^^^^^^^^^

.. code-block:: c 
    :caption: led字符设备结构体
    :linenos:

    struct led_chrdev {
    	struct cdev dev;	//描述一个字符设备的结构体
    	unsigned int __iomem *va_dr;	//数据寄存器虚拟地址指针
    	unsigned int __iomem *va_gdir;	//输入输出方向寄存器虚拟地址指针
    	unsigned int __iomem *va_iomuxc_mux;	//端口复用寄存器虚拟地址指针
    	unsigned int __iomem *va_ccm_ccgrx;	//时钟寄存器虚拟地址指针
    	unsigned int __iomem *va_iomux_pad;	//电气属性寄存器虚拟地址指针
	
    	unsigned long pa_dr;	//装载数据寄存器（物理地址）的变量
    	unsigned long pa_gdir;	//装载输出方向寄存器（物理地址）的变量
    	unsigned long pa_iomuxc_mux;	//装载端口复用寄存器（物理地址）的变量
    	unsigned long pa_ccm_ccgrx;	//装载时钟寄存器（物理地址）的变量
    	unsigned long pa_iomux_pad;	//装载电气属性寄存器（物理地址）的变量
	
    	unsigned int led_pin;	//LED的引脚
    	unsigned int clock_offset;	//时钟偏移地址（相对于CCM_CCGRx）
    };

    static struct led_chrdev led_cdev[DEV_CNT] = {
    	{.pa_dr = 0x0209C000,.pa_gdir = 0x0209C004,.pa_iomuxc_mux =
    	0x20E006C,.pa_ccm_ccgrx = 0x20C406C,.pa_iomux_pad =
    	0x20E02F8,.led_pin = 4,.clock_offset = 26},	//初始化红灯结构体成员变量
    	{.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
    	0x20E01E0,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
    	0x20E046C,.led_pin = 20,.clock_offset = 12},	//初始化绿灯结构体成员变量
    	{.pa_dr = 0x20A8000,.pa_gdir = 0x20A8004,.pa_iomuxc_mux =
    	0x20E01DC,.pa_ccm_ccgrx = 0x20C4074,.pa_iomux_pad =
    	0x20E0468,.led_pin = 19,.clock_offset = 12},	//初始化蓝灯结构体成员变量
    };

在上面的代码中我们定义了一个RGB灯的结构体，并且定义且初始化了一个RGB灯的结构体数组，
因为我们开发板上面共有3个RGB灯，所以代码中DEV_CNT为3。在初始化结构体的时候我们以“.”+“变量名字”
的形式来访问且初始化结构体变量的，初始化结构体变量的时候要以“，”隔开，使用这种方式简单明了，方便管理数据结构中的成员。


实现内核RGB模块的加载和卸载函数
^^^^^^^^^^^

.. code-block:: c 
    :caption: 内核RGB模块的加载和卸载函数
    :linenos:

    static __init int led_chrdev_init(void)
    {
    	int i = 0;
    	dev_t cur_dev;
    	
    	printk("led chrdev init\n");
    	
    	alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);	//向动态申请一个设备号
    	
    	led_chrdev_class = class_create(THIS_MODULE, "led_chrdev");	//创建设备类
    	
    	for (; i < DEV_CNT; i++) {
    		cdev_init(&led_cdev[i].dev, &led_chrdev_fops);	//绑定led_cdev与led_chrdev_fops
    		led_cdev[i].dev.owner = THIS_MODULE;
    	
    		cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);	//注册设备
    		cdev_add(&led_cdev[i].dev, cur_dev, 1);
    		device_create(led_chrdev_class, NULL, cur_dev, NULL,
    			      DEV_NAME "%d", i);	//创建设备
    	}
    	
    	return 0;
    }

    module_init(led_chrdev_init);	//模块加载
    
    static __exit void led_chrdev_exit(void)
    {
    	int i;
    	dev_t cur_dev;
    	
    	printk("led chrdev exit\n");
    	
    	for (i = 0; i < DEV_CNT; i++) {
    		cur_dev = MKDEV(MAJOR(devno), MINOR(devno) + i);	//计算出设备号
    		device_destroy(led_chrdev_class, cur_dev);	//删除设备
    		cdev_del(&led_cdev[i].dev);	//注销设备
    	}
    
    	unregister_chrdev_region(devno, DEV_CNT);	//释放被占用的设备号
    	class_destroy(led_chrdev_class);	//删除设备类
    }

    module_exit(led_chrdev_exit);	//模块卸载


第一部分为内核RGB模块的加载函数，其主要完成了以下任务：

- 调用alloc_chrdev_region()函数向系统动态申请一个未被占用的设备号，使用alloc_chrdev_region()相比较于register_chrdev_region()的好处在于不必自己费时间去查看那些是未被占用的设备号，避免了设备号重复问题；
- 调用class_create()函数创建一个RGB灯的设备类；
- 分别给三个LED建立其对应的字符设备结构体cdev和led_chrdev_fops的关联，并且初始化字符设备结构体，最后注册并创建设备。

第二部分为内核RGB模块的卸载函数，其主要完成了以下任务：

- 调用device_destroy()函数用于从linux内核系统设备驱动程序模型中移除一个设备，并删除/sys/devices/virtual目录下对应的设备目录及/dev/目录下对应的设备文件；
- 调用cdev_del()函数来释放散列表中的对象以及cdev结构本身；
- 释放被占用的设备号以及删除设备类。

从上面代代码中我们可以看出这三个LED都使用的同一个主设备号，只是他们的次设备号有所区别而已。

file_operations结构体成员函数的实现
^^^^^^^^^^^

.. code-block:: c 
    :caption: file_operations中open函数的实现
    :linenos:
    
	/* 打开RGB LED设备函数 */
    static int led_chrdev_open(struct inode *inode, struct file *filp)
    {
    	unsigned int val = 0;
    	/* 通过led_chrdev结构变量中dev成员的地址找到这个结构体变量的首地址 */
    	struct led_chrdev *led_cdev =
    	    (struct led_chrdev *)container_of(inode->i_cdev, struct led_chrdev,
    					      dev);	
    	filp->private_data = led_cdev;	//把文件的私有数据private_data指向设备结构体led_cdev
    	
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

file_operations中open函数的实现函数很重要，下面我们来详细分析一下该函数具体做了哪些工作。

1、container_of()函数:

.. image:: ./media/container_of001.PNG
   :align: center
   :alt: 找不到图片03|

在Linux驱动编程当中我们会经常和container_of()这个函数打交道，所以特意拿出来和大家分享一下，其实这个函数功能不多，但是如果单靠自己去阅读内核源代码分析，那
可能非常难以理解，编写内核源代码的大牛随便两行代码都会让我们看的云深不知处，分析内核源代码需要我们有很好的知识积累以及技术沉淀。
下面我简单跟大家讲解一下container_of()函数的大致工作内容，其宏定义实现如下所示：

.. code-block:: c 
    :caption: container_of()函数
    :linenos:

    /**
     * container_of - cast a member of a structure out to the containing structure
     *
     * @ptr:        the pointer to the member.
     * @type:       the type of the container struct this is embedded in.
     * @member:     the name of the member within the struct.
     *
     */
    #define container_of(ptr, type, member) ({                      \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
            (type *)( (char *)__mptr - offsetof(type,member) );})


该函数共有三个输入参数，分别是ptr（结构体变量中某个成员的地址）、type（结构体类型）和member（该结构体变量的具体名字），
原理其实很简单，就是通过已知类型type的成员member的地址ptr，计算出结构体type的首地址。
type的首地址 = ptr - size ，需要注意的是它们的大小都是以字节为单位计算的，container_of()函数的如下：

- 判断ptr 与 member 是否为同意类型；
- 计算size大小，结构体的起始地址 = (type *)((char *)ptr - size)  (注：强转为该结构体指针)。

通过此函数我们便可以轻松地获取led_chrdev结构体的首地址了。

2、文件私有数据:

一般很多的linux驱动都会将文件的私有数据private_data指向设备结构体，其保存了用户自定义设备结构体的地址。
自定义结构体的地址被保存在private_data后，可以通过读、写等操作通过该私有数据去访问设备结构体中的成员，
这样做体现了linux中面向对象的程序设计思想。

3、通过ioremap()函数实现地址的映射:

其实ioremap()函数我们之前分析过了，在led_chrdev_open()函数的作用都是一样的，只是分别对LED灯所用到的CCM_CCGRx时钟控制寄存器、端口复用寄存器、电气属性配置寄存器、
数据寄存器以及输入输出方向寄存器都做了地址映射，这样我们便可以通过操作程序中的虚拟地址来间接的控制物理寄存器，我们在驱动程序描述寄存器不利于驱动模块的灵活使用，
后几个章节我们会带领大家通过设备树（设备树插件）的方式去描述寄存器及其相关属性，在此先埋下伏笔，循序渐进，顺腾摸瓜，使大家能够真正理解并掌握linux驱动的精髓。

4、通过ioread32()和iowrite32()函数简介操作寄存器:

和STM32一样，都要开启I/O引脚对应的时钟、设置其端口的复用（在此复用为普通的GPIO口）、电气属性、输入输出方向以及输出的高低电平等等，
一般我们访问某个地址时都是先将该地址的数据读取到一个变量中然后修改该变量，最后再将该变量写入到原来的地址当中。
注意我们在操作这段被映射后的地址空间时应该使用linux提供的I/O访问函数（如：iowrite8()、iowrite16()、iowrite32()、
ioread8()、ioread16()、ioread32()等），这里再强调一遍，即使理论上可以直接操作这段虚拟地址了但是Linux并不建议这么做。


下面我们接着分析一下file_operations中write函数的实现：

.. code-block:: c 
    :caption: file_operations中write函数的实现
    :linenos:
    
	/* 向RGB LED设备写入数据函数 */
    static ssize_t led_chrdev_write(struct file *filp, const char __user * buf,
    				size_t count, loff_t * ppos)
    {
    	unsigned long val = 0;
    	unsigned long ret = 0;
    	int tmp = count;
    	kstrtoul_from_user(buf, tmp, 10, &ret);	//将用户空间缓存区复制到内核空间
    	struct led_chrdev *led_cdev = (struct led_chrdev *)filp->private_data;	//将文件的私有数据地址赋给led_cdev结构体指针
    	val = ioread32(led_cdev->va_dr);	//间接读取数据寄存器中的数据
    	if (ret == 0)
    		val &= ~(0x01 << led_cdev->led_pin);	//点亮LED
    	else
    		val |= (0x01 << led_cdev->led_pin);	//熄灭LED
    	iowrite32(val, led_cdev->va_dr);	//将数据重新写入寄存器中
    	*ppos += tmp;
    	return tmp;
    }

1、kstrtoul_from_user()函数:

再分析该函数之前，我们先分析一下内核中提供的kstrtoul()函数，理解kstrtoul()函数之后再分析kstrtoul_from_user()就信手拈来了。
kstrtoul()在linux4.19的include/linux/kernel.h中有如下定义。

.. code-block:: c 
    :caption: kstrtoul()函数解析
    :linenos:

    /**
     * kstrtoul - convert a string to an unsigned long
     * @s: The start of the string. The string must be null-terminated, and may also
     *  include a single newline before its terminating null. The first character
     *  may also be a plus sign, but not a minus sign.
     * @base: The number base to use. The maximum supported base is 16. If base is
     *  given as 0, then the base of the string is automatically detected with the
     *  conventional semantics - If it begins with 0x the number will be parsed as a
     *  hexadecimal (case insensitive), if it otherwise begins with 0, it will be
     *  parsed as an octal number. Otherwise it will be parsed as a decimal.
     * @res: Where to write the result of the conversion on success.
     *
     * Returns 0 on success, -ERANGE on overflow and -EINVAL on parsing error.
     * Used as a replacement for the obsolete simple_strtoull. Return code must
     * be checked.
    */
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

该函数的功能是将一个字符串转换成一个无符号长整型的数据，它一共有三个参数，各个参数详细描述如下：

- s：字符串的起始地址，该字符串必须以空字符结尾；
- base：转换基数，如果base=0，则函数会自动判断字符串的类型，且按十进制输出，比如“0xa”就会被当做十进制处理（大小写都一样），输出为10。如果是以0开头则会被解析为八进制数，否则将会被解析成小数；
- res：一个指向被转换成功后的结果的地址。

该函数转换成功后返回0，溢出将返回-ERANGE，解析出错返回-EINVAL。理解完kstrtoul()函数后想必大家已经知道kstrtoul_from_user()函数的大致用法了，
kstrtoul_from_user()函数在include/linux/kernel.h中定义如下：

.. code-block:: c 
    :caption: kstrtoul_from_user()函数
    :linenos:

    int __must_check kstrtoul_from_user(const char __user *s, size_t count, unsigned int base, unsigned long *res);

该函数相比kstrtoul()多了一个参数count，count为要转换数据的大小，因为用户空间是不可以直接访问内核空间的，所以内核提供了kstrtoul_from_user()函数以实现用户缓冲区到内核缓冲区的拷贝，与之相似的还有copy_to_user()，copy_to_user()
完成的是内核空间缓冲区到用户空io间的拷贝。如果你使用的内存类型没那么复杂，便可以选择使用put_user()或者get_user()函数。

最后我们再回到file_operations中write函数的实现中的第九行代码，该代码我们在前面已经说过了，就是将在open函数实现中存储在文件的私有数据重新拿出来用而已，后面10~15行代码便是
根据文件的私有数据来进行I/O读写访问的。


最后分析一下file_operations中release函数的实现：

.. code-block:: c 
    :caption: file_operations中release函数的实现
    :linenos:

    static int led_chrdev_release(struct inode *inode, struct file *filp)
    {
    	struct led_chrdev *led_cdev = 
			(struct led_chrdev *)container_of(inode->i_cdev, struct led_chrdev, dev);	//将文件的私有数据地址赋给led_cdev结构体指针
    	/* 释放ioremap后的虚拟地址空间 */
    	iounmap(led_cdev->va_dr);	//释放数据寄存器虚拟地址
    	iounmap(led_cdev->va_gdir);	//释放输入输出方向寄存器虚拟地址
    	iounmap(led_cdev->va_iomuxc_mux);	//释放I/O复用寄存器虚拟地址
    	iounmap(led_cdev->va_ccm_ccgrx);	//释放时钟控制寄存器虚拟地址
    	iounmap(led_cdev->va_iomux_pad);	//释放端口电气属性寄存器虚拟地址
    	return 0;
    }

当最后一个打开设备的用户进程执行close()系统调用的时候，内核将调用驱动程序release()函数，
release函数的主要任务是清理未结束的输入输出操作，释放资源，用户自定义排他标志的复位等。
前面我们用ioremap()将物理地址空间映射到了虚拟地址空间，当我们使用完该虚拟地址空间时应该记得使用iounmap()函数
将它释放掉。

LED驱动完整代码
^^^^^^^^^^^

到这里我们的代码已经分析完成了，下面时本驱动的完整代码（由于前面已经带领大家详细的分析了一遍，所以我把完整代码的注释给去掉了，希望你能够会想起每个函数的具体作用）：

.. code-block:: c 
    :caption: 完整代码
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



LED驱动Makefile
^^^^^^^^^^^

.. code-block:: makefile
    :caption: LED驱动Makefile
    :linenos:

    obj-m := led_cdev.o
    
    NATIVE ?= true
    
    ifeq ($(NATIVE), false)
    	KERNEL_DIR = /home/book/embedfire/imx6ull/linuxcore/ebf-buster-linux-master
    else
    	KERNEL_DIR = /lib/modules/$(shell uname -r)/build
    endif
    
    all:modules
    modules clean:
    	$(MAKE) -C $(KERNEL_DIR) M=$(shell pwd) $@
