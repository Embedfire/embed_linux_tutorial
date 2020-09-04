.. vim: syntax=rst



平台设备驱动
==================


在Linux设备模型章节中，我们了解到Linux中采用“总线——设备——驱动”的方式，来管理系统中的设备以及驱动。要满足Linux设备模型，就要有总线、设备和驱动。
但是有的设备并没有相对应的物理总线，比如led、rtc时钟、蜂鸣器、按键等。为了使这些设备也能够采用“总线——设备——驱动”方式进行管理，内核引入了一种虚拟的
总线——平台总线（platform bus)，用于连接那些没有物理总线的设备，而这些设备被称为平台设备，对应的设备驱动则被称为平台驱动。

对于从事Linux驱动的开发工作者来说，平台设备极为重要。
平台驱动机制将设备资源注册到的Linux内核里面，由Linux的内核统一管理，这样就可以通过统一的接口来操作设备。在更换平台的时候，
只需要修改平台驱动的设备资源配置就可以了，而不用修改驱动，具有移植性好的特点。


我们知道常见的物理总线有I2C、SPI、UART......在嵌入式开发中，我们经常使用的i2c、spi、uart，并不是真正的物理总线，
它们是集成在SOC上面的控制器，如i2c控制器，spi控制器等等，它们都可以通过CPU的总线直接寻址，也就是我们常说的操作寄存器。
对于这些控制器而言，本身也没有对应的物理总线可以进行连接，那么这些控制器也会以平台设备的身份注册到内核中去。
但我们所注册的I2C设备、SPI设备最终是会连接到I2C总线、SPI总线，而不会连接到平台总线。
简单来说，对于SOC上的i2c控制器、spi控制器是连接到平台设备总线上的，而一些I2C设备、SPI设备则是挂载到I2C总线、SPI总线上。


前面，我们已经尝试过以字符设备的方式，来点亮板子上的LED灯。因为没有采用“总线——设备——驱动”的方式，所以设备和驱动并没有做到分离，将设备的硬件信息统统都定义在了驱动代码中，
一旦硬件有所变更，就要需要修改驱动代码了。本章用平台设备驱动的方式来修改之前的代码，使得设备与驱动分开，增加驱动代码的通用性。平台设备驱动是在Linux设备模型的基础上，
对device和device_driver结构体进一步封装，得到platform device和platform driver这两个结构体，分别对应平台设备以及平台驱动。做驱动开发
只需要利用这两个结构体以及内核提供的API，便可以依葫芦画瓢，编写出平台设备驱动了。


平台设备
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

平台设备的工作是为驱动程序提供硬件资源,平台设备将设备本身的资源（如需要使用到什么寄存器，占用哪些中断号，内存资源,IO口等）
注册进内核，由内核统一管理。
平台设备最主要的一个基本特性就是可以通过CPU总线直接寻址，如我们常见的各种寄存器,而我们编写的大多数设备驱动都是为了驱动
plaftom设备,可见paltform设备对Linux驱动工程师的重要性。


内核使用platform_device结构体来描述平台设备，结构体原型如下：

.. code-block:: c
   :caption: platform_device结构体(内核源码/include/linux/platform_device.h)
   :linenos:

    struct platform_device {
        const char *name;
        int id;
        struct device dev;
        u32 num_resources;
        struct resource *resource;
        const struct platform_device_id *id_entry;
        /* 省略部分成员 */
    };


- name：设备名称，总线进行匹配时，会比较设备和驱动的名称是否一致；

- id：指定设备的编号，Linux支持同名的设备，而同名设备之间则是通过该编号进行区分；

- dev：Linux设备模型中的device结构体，platform_device中嵌入继承了该结构体，方便内核管理平台设备；

- num_resources：记录资源的个数，当结构体成员resource存放的是数组时，需要记录resource数组的个数，内核提供了宏定义ARRAY_SIZE用于计算数组的个数；

- resource：平台设备提供给驱动的资源，如irq，dma，内存等等。该结构体会在接下来的内容进行讲解；

- id_entry：平台总线提供的另一种匹配方式，原理依然是通过比较字符串，这部分内容会在平台总线小节中讲，这里的id_entry用于保存匹配的结果；


在平台设备结构体中，使用结构体struct resource来保存设备所提供的资源，比如设备使用的中断编号，寄存器物理地址等，结构体原型如下：


.. code-block:: c
    :caption: resource结构体(内核源码/include/linux/ioport.h)
    :linenos:

    /*
    * Resources are tree-like, allowing
    * nesting etc..
    */

    struct resource {
        resource_size_t start;
        resource_size_t end;
        const char *name;
        unsigned long flags;
        /* 省略部分成员 */
    };


- name：指定资源的名字，可以设置为NULL；

- start、end：指定资源的起始地址以及结束地址

- flags：用于指定该资源的类型，在Linux中，资源包括I/O、Memory、Register、IRQ、DMA、Bus等多种类型，最常见的有以下几种：

.. code-block:: c
    :caption: 资源宏定义(内核源码/include/linux/ioport.h)
    :linenos:

    #define IORESOURCE_IO   0x00000100
    #define IORESOURCE_MEM  0x00000200
    #define IORESOURCE_IRQ  0x00000400
    #define IORESOURCE_DMA  0x00000800


设备驱动程序的主要目的是操作设备的寄存器。不同架构的计算机提供不同的操作接口，主要有IO端口映射和IO內存映射两种方式。
IORESOURCE_IO指的是IO地址空间，对应于IO端口映射方式，只能通过专门的接口函数（如inb、outb）才能访问；
IORESOURCE_MEM指的是属于外设的可直接寻址的地址空间，也就是我们常说的某个寄存器地址，
采用IO内存映射的方式，可以像访问内存一样，去读写寄存器。在嵌入式中，基本上没有IO地址空间，所以通常使用IORESOURCE_MEM。
IORESOURCE_IRQ可以指定该设备使用某个中断，而IORESOURCE_DMA则是用于指定使用的DMA通道。

在资源的起始地址和结束地址中，对于IORESOURCE_IO或者是IORESOURCE_MEM，他们表示要使用的内存的起始位置以及结束位置；
而对于IORESOURCE_IRQ、IORESOURCE_DMA，若是只用一个中断引脚或者是一个通道，则start和end成员的值必须是相等的。


Linux虽然提供了很多种资源类型供我们选择，但是不一定能够囊括所有的数据，如某个GPIO的引脚号。尽管如此，我们依然有办法可以解决这个问题。以上述问题为例，我们可以使用IORESOURCE_IRQ资源，把该引脚编号赋给start和end成员，驱动只需要调用对应的API，便可以得到我们的引脚号。这只是其中的一种方式，
常见的方式是以下这种使用方式。我们注意到platform_device结构体中，有个device结构体类型的成员dev。上一章，我们提到过Linux设备模型使用device结构体来抽象物理设备，
该结构体的成员platform_data可用于保存设备的私有数据，于是，我们便可以利用该成员做文章，这样的话，无论你想要提供的是什么内容，只需要把数据的地址赋值给platform_data即可，还是以GPIO引脚号为例，示例代码如下：


.. code-block:: c
    :caption: 示例代码
    :linenos: 

    unsigned int pin = 10;

    struct platform_device pdev = {
        .dev = {
            .platform_data = &pin;
        }
    }


将保存了GPIO引脚号的变量地址赋值给platform_data成员，这样，可以通过调用特定的API，获取到我们需要的引脚号。

当我们完成了上述结构体的初始化时，需要告诉内核，我们定义了一个平台设备。为此，需要使用下面的API，来注册平台设备。

.. code-block:: c
    :caption: platform_device_register函数(内核源码/drivers/base/platform.c)
    :linenos:

    int platform_device_register(struct platform_device *pdev)



同样，当我们想要移除我们的平台设备时，我们需要使用platform_device_unregister函数，来通知内核去移除该设备。



.. code-block:: c 
    :caption: platform_device_unregister函数(内核源码/drivers/base/platform.c)
    :linenos:

    void platform_device_unregister(struct platform_device *pdev)

到这里，平台设备的知识已经讲解完毕，平台设备的主要内容是将硬件部分的代码与驱动部分的代码分开，注册到平台设备总线中，在设备与驱动中间搭建
了一座桥——统一的数据结构以及函数接口，设备和驱动的数据交互直接在“这座桥上”进行。


平台驱动
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

现在我们想象一下，已经在内核中注册了一个平台设备，而我们的驱动程序应该如何去配合平台设备使得我们的设备正常工作呢，这便是平台驱动的重点。
内核中使用platform_driver结构体来描述平台驱动，结构体原型如下所示：



.. code-block:: c
    :caption: platform_driver结构体(内核源码/include/platform_device.h)
    :linenos:

    struct platform_driver {

        int (*probe)(struct platform_device *);
        int (*remove)(struct platform_device *);
        struct device_driver driver;
        const struct platform_device_id *id_table;

    };



- probe：函数指针类型，指向驱动的probe函数，当总线为设备和驱动匹配上之后，会执行驱动的probe函数。我们通常在该函数中，对设备进行一系列的初始化。

- remove：函数指针类型，指向驱动的remove函数，当我们移除我们的平台设备时，会调用该函数，该函数实现的操作，通常是probe函数的逆过程。

- driver：Linux设备模型中用于抽象驱动的device_driver结构体，platform_driver嵌入该结构体，方便内核管理平台驱动；

- id_table：表示该驱动能够兼容的设备类型。



platform_device_id结构体原型如下所示:

.. code-block:: c
    :caption: id_table结构体(内核源码/include/linux/mod_devicetable.h)
    :linenos:

    struct platform_device_id {
        char name[PLATFORM_NAME_SIZE];
        kernel_ulong_t driver_data;

    };


在platform_device_id这个结构体中，有两个成员，第一个是数组用于指定驱动的名称，总线进行匹配时，会依据该结构体的name成员与platform_device中的变量name进行比较匹配，
另一个成员变量driver_data，则是用于来保存设备的配置。我们知道在同系列的设备中，往往只是某些寄存器的配置不一样，为了减少代码的冗余，
尽量做到一个驱动可以匹配多个设备的目的。接下来以imx的串口为例，具体看下这个结构体的作用：


.. code-block:: c
    :caption: 示例代码(内核源码/drivers/tty/serial/imx.c)
    :linenos:

    static struct imx_uart_data imx_uart_devdata[] = {

        [IMX1_UART] = {
            .uts_reg = IMX1_UTS,
            .devtype = IMX1_UART,
        },

        [IMX21_UART] = {
            .uts_reg = IMX21_UTS,
            .devtype = IMX21_UART,
        },

        [IMX6Q_UART] = {
            .uts_reg = IMX21_UTS,
            .devtype = IMX6Q_UART,
        },

    };


    static struct platform_device_id imx_uart_devtype[] = {

        {
            .name = "imx1-uart",
            .driver_data = (kernel_ulong_t) &imx_uart_devdata[IMX1_UART],
        }, 

        {
            .name = "imx21-uart",
            .driver_data = (kernel_ulong_t) &imx_uart_devdata[IMX21_UART],
        }, 

        {
            .name = "imx6q-uart",
            .driver_data = (kernel_ulong_t) &imx_uart_devdata[IMX6Q_UART],

        }, 
        
        {
            /* sentinel */

        }

    };

在上面的代码中，支持三种设备的串口，支持imx1、imx21、imx6q三种不同系列芯片，他们之间区别在于串口的test寄存器地址不同。
当总线成功配对平台驱动以及平台设备时，会将对应的id_table条目赋值给平台设备的id_entry成员，而平台驱动的probe函数是以平台设备为参数，
这样的话，就可以拿到当前设备串口的test寄存器地址了。


当我们初始化了平台驱动结构体之后，通过以下函数来注册我们的平台驱动，由于platform_driver中嵌入了driver结构体，结合Linux设备模型的知识，
那么当我们成功注册了一个平台驱动时，就会在/sys/bus/platform/driver目录生成一个新的子目录。

.. code-block:: c 
    :caption: platform_driver_register函数
    :linenos:

    int platform_driver_register(struct platform_driver *drv);


当我们移除我们的模块时，需要注销掉已注册的平台驱动，Linux提供以下函数，用于注销我们的平台驱动。

.. code-block:: c 
    :caption: platform_driver_unregister函数(内核源码/drivers/base/platform.c)
    :linenos:

    void platform_driver_unregister(struct platform_driver *drv);



上面所讲的内容是最基本的平台驱动框架，只需要实现probe函数、remove函数，初始化platform_driver结构体，并调用platform_driver_register进行注册即可。


这只是完成了本小节的一个重点，另一个重点便是如何获取平台设备提供的资源。在学习平台设备的时候，我们知道Linux使用结构体resource来抽象我们的资源，
以及可以利用设备结构体device中的成员platform_data来保存私有数据。下面，先看一下，如何获取平台设备中结构体resource提供的资源。
函数platform_get_resource通常会在驱动的probe函数中执行，用于获取平台设备提供的资源结构体，最终会返回一个struct resource类型的指针，

函数原型如下：

.. code-block:: c
    :caption: platform_get_resource函数
    :linenos:

    struct resource *platform_get_resource(struct platform_device *dev, unsigned int type, unsigned int num);



- dev：指定要获取哪个平台设备的资源；

- type：指定获取资源的类型，如IORESOURCE_MEM、IORESOURCE_IO等；

- num：指定要获取的资源编号。每个设备所需要资源的个数是不一定的，为此内核对这些资源进行了编号，对于不同的资源，编号之间是相互独立的。



假若资源类型为IORESOURCE_IRQ，内核还提供以下函数接口，来获取中断引脚，



.. code-block:: c 
    :caption: platform_get_irq函数
    :linenos:

    int platform_get_irq(struct platform_device *pdev, unsigned int num)



- pdev：指定要获取哪个平台设备的资源；

- num：指定要获取的资源编号。



对于存放在device结构体中成员platform_data的数据，我们可以使用dev_get_platdata函数来获取，函数原型如下所示：



.. code-block:: c 
    :caption: dev_get_platdata函数
    :linenos:


    static inline void *dev_get_platdata(const struct device *dev)
    {
        return dev->platform_data;
    }


dev_get_platdata函数的实现十分简单，直接返回device结构体中成员platform_data的值。


以上几个函数接口就是如何从平台设备中获取资源的常用的几个函数接口，到这里平台驱动部分差不多就结束了。总结一下平台驱动需要
实现probe函数，当平台总线成功匹配驱动和设备时，则会调用驱动的probe函数，在该函数中使用上述的函数接口来获取资源，
以初始化设备，最后填充结构体platform_driver，调用platform_driver_register进行注册。



平台总线
~~~~~~~~~~~~~~~~~~~~~~~~~~~

在Linux的设备驱动模型中，总线是最重要的一环。上一节中，我们提到过总线是负责匹配设备和驱动，
它维护着两个链表，里面记录着各个已经注册的平台设备和平台驱动。每当有新的设备或者是新的驱动加入到总线时，
总线便会调用platform_match函数对新增的设备或驱动，进行配对。内核中使用bus_type来抽象描述系统中的总线，平台总线结构体原型如下所示：


.. code-block:: c
    :caption: platform_bus_type结构体(内核源码/driver/base/platform.c)
    :linenos:

    struct bus_type platform_bus_type = {

        .name		= "platform",
        .dev_groups	= platform_dev_groups,
        .match		= platform_match,
        .uevent		= platform_uevent,
        .pm		= &platform_dev_pm_ops,

    };

    EXPORT_SYMBOL_GPL(platform_bus_type);


内核用platform_bus_type来描述平台总线，由于内核已经替我们实现了平台总线，所以这边我们只需要了解platform总线的match函数，
清楚platform总线是如何将平台设备以及平台驱动联系到一起即可，其函数原型如下：


.. code-block:: c
    :caption: platform_match函数(内核源码/driver/base/platform.c)
    :linenos:

    static int platform_match(struct device *dev, struct device_driver *drv)
    {

        struct platform_device *pdev = to_platform_device(dev);
        struct platform_driver *pdrv = to_platform_driver(drv);

        /* When driver_override is set, only bind to the matching driver */
        if (pdev->driver_override)
            return !strcmp(pdev->driver_override, drv->name);

        /* Attempt an OF style match first */
        if (of_driver_match_device(dev, drv))
            return 1;

        /* Then try ACPI style match */
        if (acpi_driver_match_device(dev, drv))
            return 1;

        /* Then try to match against the id table */
        if (pdrv->id_table)
            return platform_match_id(pdrv->id_table, pdev) != NULL;

        /* fall-back to driver name match */
        return (strcmp(pdev->name, drv->name) == 0);

    }



platform_match函数只传入两个参数：dev和drv。我们知道在platform_device和platform_driver中也有对应的device、device_driver类型成员，
在platform_match开头，调用了两个宏定义to_platform_device和to_platform_driver，原型如下所示：


.. code-block:: c
    :caption: to_platform_xxx宏定义(内核源码/include/linux/platform_device.h)
    :linenos:

    #define to_platform_device(x)     (container_of((x), struct platform_device, dev)
    #define to_platform_driver(drv)   (container_of((drv), struct platform_driver, driver))


宏定义to_platform_device和to_platform_driver实现了对container_of的封装，利用该这两个宏便可以得到进行匹配的platform_driver和platform_device。
platform总线提供了四种匹配方式，并且这四种方式存在着优先级：设备树机制>ACPI匹配模式>id_table方式>字符串比较。虽然匹配方式五花八门，但是并没有涉及到任何复杂的算法，
都只是在匹配的过程中，比较一下设备和驱动提供的某个成员的字符串是否相同。
设备树是一种描述硬件的数据结构，它用一个非C语言的脚本来描述这些硬件设备的信息。驱动和设备之间的匹配时通过比较compatible的值。
acpi主要是用于电源管理，基本上用不到，这里就并不进行讲解了。关于设备树的匹配机制，会在设备树章节进行详细分析。
我们在定义结构体platform_driver时，我们需要提供一个id_table的数组，该数组说明了当前的驱动能够支持的设备。当加载该驱动时，总线的match函数发现id_table非空，
则会比较id_table中的name成员和平台设备的name成员，若相同，则会返回匹配的条目，具体的实现过程如下：



.. code-block:: c
    :caption: platform_match_id函数(内核源码/drivers/base/platform.c)
    :linenos:

    static const struct platform_device_id *platform_match_id(
                const struct platform_device_id *id,
                struct platform_device *pdev)

    {
        while (id->name[0]) {
            if (strcmp(pdev->name, id->name) == 0) {
                pdev->id_entry = id;
                return id;
            }
            id++;
        }
        return NULL;
    }



每当有新的驱动或者设备添加到总线时，总线便会调用match函数对新的设备或者驱动进行配对。platform_match_id函数中第一个参数为驱动提供的id_table，
第二个参数则是待匹配的平台设备。当待匹配的平台设备的name字段的值等于驱动提供的id_table中的值时，会将当前匹配的项赋值给platform_device中的id_entry，
返回一个非空指针。若没有成功匹配，则返回空指针。



.. image:: ./media/id_table_match.jpg
   :align: center
   :alt: 驱动和设备匹配过程



倘若我们的驱动没有提供前三种方式的其中一种，那么总线进行匹配时，只能比较platform_device中的name字段以及嵌在platform_driver中的device_driver的name字段。


.. image:: ./media/name_match.jpg
   :align: center
   :alt: 名称匹配方式




实验
~~~~~~~~~~~~~

前面的小节，学习了平台设备驱动的相关理论知识。回到我们最初的问题，本节将会把平台设备驱动应用到LED字符设备驱动的代码中，实现硬件与软件代码相分离，
巩固平台设备驱动的学习。

**本章的示例代码目录为：base_code/linux_driver/platform_driver**

定义平台设备
---------------

我们需要将字符设备中的硬件信息提取出来，独立成一份代码，将其作为平台设备，注册到内核中。点亮LED灯，需要控制与LED灯相关的寄存器，
包括GPIO时钟寄存器，IO配置寄存器，IO数据寄存器等，这里的资源，实际上就是寄存器地址，可以使用IORESOURCE_MEM进行处理；
除了这些之外，还需要提供一些寄存器的偏移量，我们可以利用平台设备的私有数据进行管理。


.. code-block:: c
    :caption: 寄存器宏定义(位于../base_code/linux_driver/platform_driver/led_pdev.c)
    :linenos:

    #define CCM_CCGR1                               0x020C406C	//时钟控制寄存器
    #define IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04        0x020E006C	//GPIO1_04复用功能选择寄存器
    #define IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04        0x020E02F8	//PAD属性设置寄存器
    #define GPIO1_GDIR                              0x0209C004	//GPIO方向设置寄存器（输入或输出）
    #define GPIO1_DR                                0x0209C000	//GPIO输出状态寄存器

    #define CCM_CCGR3                               0x020C4074
    #define GPIO4_GDIR                              0x020A8004
    #define GPIO4_DR                                0x020A8000

    #define IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO020       0x020E01E0
    #define IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO020       0x020E046C

    #define IOMUXC_SW_MUX_CTL_PAD_GPIO4_IO019       0x020E01DC
    #define IOMUXC_SW_PAD_CTL_PAD_GPIO4_IO019       0x020E0468


关于LED灯的寄存器，我们采用宏定义进行封装，具体每个寄存器的作用，可以参考《IMX6ULRM》用户手册。
定义一个resource结构体，用于存放上述的寄存器地址，提供给驱动使用，如下所示：


.. code-block:: c
    :caption: 定义资源数组(位于../base_code/linux_driver/platform_driver/led_pdev.c)
    :linenos: 

    static struct resource rled_resource[] = {
        [0] = DEFINE_RES_MEM(GPIO1_DR, 4),
        [1] = DEFINE_RES_MEM(GPIO1_GDIR, 4),
        [2] = DEFINE_RES_MEM(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO04, 4),
        [3] = DEFINE_RES_MEM(CCM_CCGR1, 4),
        [4] = DEFINE_RES_MEM(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO04, 4),
    };



在内核源码/include/linux/ioport.h中，提供了宏定义DEFINE_RES_MEM、DEFINE_RES_IO、DEFINE_RES_IRQ和DEFINE_RES_DMA，用来定义所需要的资源类型。
DEFINE_RES_MEM用于定义IORESOURCE_MEM类型的资源，我们只需要传入两个参数，一个是寄存器地址，另一个是大小。从手册上看，可以得知一个寄存器都是32位的，因此，
这里我们选择需要4个字节大小的空间。rled_resource资源数组中，我们将所有的MEM资源进行了编号，0对应了GPIO1_DR，1对应了GPIO1_GDIR，驱动到时候就可以根据这些编号获得对应的寄存器地址。

下面我们使用一个数组rled_hwinfo，来记录寄存器的偏移量，填充平台私有数据时，只需要把数组的首地址赋给platform_data即可。


.. code-block:: c
    :caption: 定义平台设备的私有数据(位于../base_code/linux_driver/platform_driver/led_pdev.c)
    :linenos: 

    unsigned int rled_hwinfo[2] = { 4, 26 };


关于设备的硬件信息，我们已经全部完成了，接下来只需要定义一个platform_device类型的变量，填充相关信息。


.. code-block:: c
    :caption: 定义平台设备(位于../base_code/linux_driver/platform_driver/led_pdev.c)
    :linenos: 

    static int led_cdev_release(struct inode *inode, struct file *filp)
    {
        return 0;
    }

    /* red led device */ 
    static struct platform_device rled_pdev = {
        .name = "led_pdev",
        .id = 0,
        .num_resources = ARRAY_SIZE(rled_resource),
        .resource = rled_resource,
        .dev = {
            .release = led_release,
            .platform_data = rled_hwinfo,
            },

    };



这里我们定义了一个设备名为“led_pdev”的设备，这里的名字确保要和驱动的名称保持一致，否则就会导致匹配失败。id编号设置为0，驱动会利用该编号来注册设备。
对于设备资源，我们将上面实现好的rled_resource数组赋值给resource成员，同时，我们还需要指定资源的数量，内核提供了宏定义ARRAY_SIZE，用于计算数组长度，
因此，num_resources直接赋值为ARRAY_SIZE(rled_resource)。这里的led_release函数为空，目的为了防止卸载模块，内核提示报错。


最后，只需要在模块加载的函数中调用platform_device_register函数，这样，当加载该内核模块时，新的平台设备就会被注册到内核中去，实现方式如下：


.. code-block:: c
    :caption: 模块初始化(位于../base_code/linux_driver/platform_driver/led_pdev.c)
    :linenos:


    static __init int led_pdev_init(void)
    {
        printk("pdev init\n");
        platform_device_register(&rled_pdev);
        return 0;

    }
    module_init(led_pdev_init);


    static __exit void led_pdev_exit(void)
    {
        printk("pdev exit\n");
        platform_device_unregister(&rled_pdev);

    }
    module_exit(led_pdev_exit);


    MODULE_AUTHOR("Embedfire");
    MODULE_LICENSE("GPL");
    MODULE_DESCRIPTION("the example for platform driver");


这样，我们就实现了一个新的设备，只需要在开发板上加载该模块，平台总线下就会挂载我们LED灯的平台设备。



定义平台驱动
-------------------

我们已经注册了一个新的平台设备，驱动只需要提取该设备提供的资源，并提供相应的操作方式即可。这里我们仍然采用字符设备来控制我们的LED灯，
想必大家对于LED灯字符设备的代码已经很熟悉了，对于这块的代码就不做详细介绍了，让我们把重点放在平台驱动上。

我们驱动提供id_table的方式，来匹配设备。我们定义一个platform_device_id类型的变量led_pdev_ids，说明驱动支持哪些设备，
这里我们只支持一个设备，名称为led_pdev，要与平台设备提供的名称保持一致。

.. code-block:: c
    :caption: id_table(位于../base_code/linux_driver/platform_driver/led_pdrv.c)
    :linenos: 


    static struct platform_device_id led_pdev_ids[] = {
        {.name = "led_pdev"},
        {}
    };
    MODULE_DEVICE_TABLE(platform, led_pdev_ids);


这块代码提供了驱动支持哪些设备，这仅仅完成了第一个内容，这是总线进行匹配时所需要的内容。而在匹配成功之后，驱动需要去提取设备的资源，
这部分工作都是在probe函数中完成。由于我们采用字符设备的框架，因此，在probe过程，还需要完成字符设备的注册等工作，具体实现的代码如下：

.. code-block:: c
    :caption: led_pdrv_probe函数(位于../base_code/linux_driver/platform_driver/led_pdrv.c)
    :linenos: 

    struct led_data {
        unsigned int led_pin;
        unsigned int clk_regshift;

        unsigned int __iomem *va_dr;
        unsigned int __iomem *va_gdir;
        unsigned int __iomem *va_iomuxc_mux;
        unsigned int __iomem *va_ccm_ccgrx;
        unsigned int __iomem *va_iomux_pad;	

        struct cdev led_cdev;
    };    


    static int led_pdrv_probe(struct platform_device *pdev)
    {
        struct led_data *cur_led;
        unsigned int *led_hwinfo;

        
        struct resource *mem_dr;
        struct resource *mem_gdir;
        struct resource *mem_iomuxc_mux;
        struct resource *mem_ccm_ccgrx;
        struct resource *mem_iomux_pad; 	

        dev_t cur_dev;
        int ret = 0;

        printk("led platform driver probe\n");

        //第一步：提取平台设备提供的资源
        cur_led = devm_kzalloc(&pdev->dev, sizeof(struct led_data), GFP_KERNEL);
        if(!cur_led)
            return -ENOMEM;

        led_hwinfo = devm_kzalloc(&pdev->dev, sizeof(unsigned int)*2, GFP_KERNEL);
        if(!led_hwinfo)
            return -ENOMEM;

        /* get the pin for led and the reg's shift */
        led_hwinfo = dev_get_platdata(&pdev->dev);
        cur_led->led_pin = led_hwinfo[0];
        cur_led->clk_regshift = led_hwinfo[1];

        /* get platform resource */
        mem_dr = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        mem_gdir = platform_get_resource(pdev, IORESOURCE_MEM, 1);
        mem_iomuxc_mux = platform_get_resource(pdev, IORESOURCE_MEM, 2);
        mem_ccm_ccgrx = platform_get_resource(pdev, IORESOURCE_MEM, 3);
        mem_iomux_pad = platform_get_resource(pdev, IORESOURCE_MEM, 4);



        cur_led->va_dr =
            devm_ioremap(&pdev->dev, mem_dr->start, resource_size(mem_dr));

        cur_led->va_gdir =
            devm_ioremap(&pdev->dev, mem_gdir->start, resource_size(mem_gdir));

        cur_led->va_iomuxc_mux =
            devm_ioremap(&pdev->dev, mem_iomuxc_mux->start,resource_size(mem_iomuxc_mux));

        cur_led->va_ccm_ccgrx =
            devm_ioremap(&pdev->dev, mem_ccm_ccgrx->start,
                resource_size(mem_ccm_ccgrx));

        cur_led->va_iomux_pad =
            devm_ioremap(&pdev->dev, mem_iomux_pad->start,resource_size(mem_iomux_pad));

        //第二步：注册字符设备
        cur_dev = MKDEV(DEV_MAJOR, pdev->id);

        register_chrdev_region(cur_dev, 1, "led_cdev");
        cdev_init(&cur_led->led_cdev, &led_cdev_fops);

        ret = cdev_add(&cur_led->led_cdev, cur_dev, 1);
        if(ret < 0)
        {
            printk("fail to add cdev\n");
            goto add_err;
        }

        device_create(my_led_class, NULL, cur_dev, NULL, DEV_NAME "%d", pdev->id);

        /* save as drvdata */ 
        platform_set_drvdata(pdev, cur_led);
        return 0;

    add_err:
        unregister_chrdev_region(cur_dev, 1);
        return ret;

    }

- 代码1-12行，仍然使用结构体led_data来管理我们LED灯的硬件信息
- 代码31-38行，使用devm_kzalloc函数申请cur_led和led_hwinfo结构体内存大小
- 代码41-43行，使用dev_get_platdata函数获取私有数据，得到LED灯的寄存器偏移量，并赋值给cur_led->led_pin和cur_led->clk_regshift。
- 代码46-50行，利用函数platform_get_resource可以获取到各个寄存器的地址
- 代码52-66行，在内核中，这些地址并不能够直接使用，使用devm_ioremap将获取到的寄存器地址转化为虚拟地址，到这里我们就完成了提取资源的工作了


接下来，就需要注册一个LED字符设备了。开发板上板载了三个LED灯，在rled_pdev结构体中，我们指定了红灯的ID号为0，我们可以利用该id号，
来作为字符设备的次设备号，用于区分不同的LED灯。使用MKDEV宏定义来创建一个设备编号，再调用register_chrdev_region、cdev_init、cdev_add等函数来注册字符设备。
在probe函数的最后，我们使用platform_set_drvdata函数，将LED数据信息存入在平台驱动结构体中pdev->dev->driver_data中。


当驱动的内核模块被卸载时，我们需要将注册的驱动注销，相应的字符设备也同样需要注销，具体的实现代码如下：

.. code-block:: c
    :caption: led_pdrv_remove函数(位于../base_code/linux_driver/platform_driver/led_pdrv.c)
    :linenos: 

    static int led_pdrv_remove(struct platform_device *pdev)
    {
        dev_t cur_dev; 
        struct led_data *cur_data = platform_get_drvdata(pdev);

        printk("led platform driver remove\n");

        cur_dev = MKDEV(DEV_MAJOR, pdev->id);
        cdev_del(&cur_data->led_cdev);
        device_destroy(my_led_class, cur_dev);
        unregister_chrdev_region(cur_dev, 1);

        return 0;

    }



我们在probe函数中调用了platform_set_drvdata，将当前的LED灯数据结构体保存到pdev的driver_data成员中，
我们只需要调用platform_get_drvdata，即可获取当前LED灯对应的结构体，该结构体中包含了字符设备，调用cdev_del删除对应的字符设备，
删除/dev目录下的设备，则调用函数device_destroy，最后使用函数unregister_chrdev_region，注销掉当前的字符设备编号



关于操作LED灯字符设备的方式，实现方式如下，具体介绍可以参阅LED灯字符设备章节的内容。



.. code-block:: c
    :caption: led灯的字符设备框架(位于../base_code/linux_driver/platform_driver/led_pdrv.c)
    :linenos: 

    static int led_cdev_open(struct inode *inode, struct file *filp)
    {
        printk("%s\n", __func__);

        struct led_data *cur_led = container_of(inode->i_cdev, struct led_data, led_cdev);
        unsigned int val = 0;


        val = readl(cur_led->va_ccm_ccgrx);
        val &= ~(3 << cur_led->clk_regshift);
        val |= (3 << cur_led->clk_regshift);
        writel(val, cur_led->va_ccm_ccgrx);

        writel(5, cur_led->va_iomuxc_mux);
        writel(0x1F838, cur_led->va_iomux_pad);

        val = readl(cur_led->va_gdir);
        val &= ~(1 << cur_led->led_pin);
        val |= (1 << cur_led->led_pin);
        writel(val, cur_led->va_gdir);


        val = readl(cur_led->va_dr);
        val |= (0x01 << cur_led->led_pin);
        writel(val, cur_led->va_dr);

        filp->private_data = cur_led;

        return 0;

    }

    static int led_cdev_release(struct inode *inode, struct file *filp)
    {
        return 0;
    }


    static ssize_t led_cdev_write(struct file *filp, const char __user * buf,
                    size_t count, loff_t * ppos)
    {
        unsigned long val = 0;
        unsigned long ret = 0;
        int tmp = count;

        struct led_data *cur_led = (struct led_data *)filp->private_data;
        kstrtoul_from_user(buf, tmp, 10, &ret);
        val = readl(cur_led->va_dr);

        if (ret == 0)
            val &= ~(0x01 << cur_led->led_pin);
        else
            val |= (0x01 << cur_led->led_pin);

        writel(val, cur_led->va_dr);
        *ppos += tmp;
        return tmp;

    }

    static struct file_operations led_cdev_fops = {
        .open = led_cdev_open,
        .release = led_cdev_release,
        .write = led_cdev_write,

    };



最后，我们只需要将我们实现好的内容，填充到platform_driver类型的结构体，并使用platform_driver_register函数注册即可。


.. code-block:: c
    :caption: 注册平台驱动(位于../base_code/linux_driver/platform_driver/led_pdrv.c)
    :linenos: 

    static struct platform_driver led_pdrv = {    
        .probe = led_pdrv_probe,
        .remove = led_pdrv_remove,
        .driver.name = "led_pdev",
        .id_table = led_pdev_ids,
    };


    static __init int led_pdrv_init(void)
    {

        printk("led platform driver init\n");

        my_led_class = class_create(THIS_MODULE, "my_leds");
        platform_driver_register(&led_pdrv);

        return 0;
    }
    module_init(led_pdrv_init);


    static __exit void led_pdrv_exit(void)
    {
        printk("led platform driver exit\n");	

        platform_driver_unregister(&led_pdrv);
        class_destroy(my_led_class);
    }
    module_exit(led_pdrv_exit);


    MODULE_AUTHOR("Embedfire");
    MODULE_LICENSE("GPL");
    MODULE_DESCRIPTION("the example for platform driver");



我们在led_pdrv中定义了两种匹配模式，在平台总线匹配过程中，只会根据id_table中的name值进行匹配，若和平台设备的name值相等，则表示匹配成功；
反之，则匹配不成功，表明当前内核没有该驱动能够支持的设备。在模块的初始化函数led_pdrv_init中，我们调用函数class_create，来创建一个led类，并且调用函数platform_driver_register，
注册我们的平台驱动结构体，这样当加载该内核模块时，就会有新的平台驱动加入到内核中。模块的注销函数led_pdrv_exit，则是初始化函数的逆过程。


编译led_pdrv.c和led_pdev.c的Makefile如下所示，编写该Makefile时，只需要根据实际情况修改变量KERNEL_DIR和obj-m即可。

.. code-block:: makefile
   :caption: Makefile(位于../base_code/linux_driver/platform_driver/Makefile)
   :linenos:  

    KERNEL_DIR = /home/embedfire/linux4.19

    obj-m := led_pdev.o led_pdrv.o

    all:modules
    modules clean:
        $(MAKE) -C $(KERNEL_DIR) M=$(shell pwd) $@



实验结果
-------------------

教程中为了节省篇幅，只列举了一个led灯，配套的例程中提供了三个LED的代码。当我们运行命令“insmod led_pdev.ko”后，
可以在/sys/bus/platform/devices下看到我们注册的LED灯设备，共有三个，后面的数字0、1、2对应了平台设备结构体的id编号。


.. image:: ./media/led_devices.jpg
   :align: center
   :alt: led灯设备


执行命令“insmod led_pdrv.ko”，加载LED的平台驱动。在运行命令“dmesg|tail"来查看内核打印信息，可以看到打印了三次probe，分别对应了三个LED灯设备。

.. image:: ./media/result.jpg
   :align: center
   :alt: led灯设备


通过驱动代码，最后会在/dev下创建三个LED灯设备，分别为led0、led1、led2，可以使用echo命令来测试我们的LED驱动是否正常。
以红灯（/dev/led0）为例，我们使用命令“echo 0 > /dev/led0”可控制红灯亮，命令“echo 1 > /dev/led0”可控制红灯亮，