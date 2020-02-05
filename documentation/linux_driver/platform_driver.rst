.. vim: syntax=rst

平台设备驱动
======
对于USB，I2S，I2C，UART，SPI等物理总线，我们都并不陌生。而像i2c设备、usb设备、spi设备等等，
都是直接挂在对应的总线下，与cpu进行数据交互的。但是在嵌入式系统当中，并不是所有的设备都能有对应
的总线。为了让这些没有总线可依靠的设备，能够满足“总线-设备-驱动”的驱动模型，Linux设备驱动模型虚
构出一条总线——平台总线，它用于挂载那些不依赖于物理总线的设备。我们经常接触到i.MX6的SPI、
I2C、UART并不是实际的物理总线，而是叫SPI控制器、I2C控制器、UART控制器，也是属
于一种硬件设备，对于Linux内核而言，属于平台设备，并挂载在平台总线下。


平台总线
~~~~~

在Linux的设备驱动模型中，总线是最重要的一环。它维护了一个链表，里面记录着各个已经注册的平台设备和平台驱动。每当有新的设备或者是新的驱动加入到总线时，
总线便会调用platform_match函数对新增的设备或驱动，进行配对。

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


平台设备
~~~~~

结构体
----
内核使用struct platform_device来表示平台设备，如下所示（删掉了一些成员变量）：

.. code-block:: c
    :caption: platform_device结构体(内核源码/include/linux/platform_device.h)
    :linenos:

    struct platform_device {
	const char	*name;
	int		id;
	struct device	dev;
	u32		num_resources;
	struct resource	*resource;
    };

- name：设备的名称，总线进行匹配时，是通过比较设备和驱动的名称，因此必须保证设备和驱动的名称是完全一致的。
- id：
- dev：
- num_resources：记录资源的个数，当结构体成员resource存放的是数组时，需要记录resource数组的个数，内核提供了宏定义ARRAY_SIZE用于计算数组的个数。
- resource：平台设备提供给内核驱动的资源，如irq，dma，内存等等。该结构体会在接下来的内容进行讲解。

注册/移除平台设备
----
当我们完成了上述结构体的初始化时，需要告诉内核，我们定义了一个平台设备。为此，需要使用下面的API，来注册平台设备。


.. code-block:: c
    :caption: platform_device_register函数(内核源码/drivers/base/platform.c)
    :linenos:

    int platform_device_register(struct platform_device *pdev)
    {
        device_initialize(&pdev->dev);
        arch_setup_pdev_archdata(pdev);
        return platform_device_add(pdev);
    }
    EXPORT_SYMBOL_GPL(platform_device_register);

同样，当我们想要移除我们的平台设备时，我们需要使用platform_device_unregister函数，来通知内核去移除该设备。

.. code-block:: c 
    :caption: platform_device_unregister函数(内核源码/drivers/base/platform.c)
    :linenos:

    void platform_device_unregister(struct platform_device *pdev)
    {
        platform_device_del(pdev);
        platform_device_put(pdev);
    }
    EXPORT_SYMBOL_GPL(platform_device_unregister);

资源
----

对于平台设备而言，内核对于该设备一无所知。为此，在我们定义平台设备时，
往往需要提供一些资源，比如这个设备使用的中断编号，寄存器的内存地址等等，这样的话，内核驱动就知道，如何使这个设备正常工作了。
平台设备向设备驱动提供资源的方式有两种：一、通过内核提供的资源类型，共有六种；二、我们自定义的数据类型，即私有数据。

内核提供的资源
^^^^^^^^^^
接触过单片机的读者，应该都知道：想要设备能够正常工作，需要对设备的寄存器以及中断信号进行设置。对于Linux而言，也不外如此。
Linux用资源来描述一个设备正常工作所需要的元素，比如IRQ，MEM，DMA等。内核提供了六种类型资源：

.. code-block:: c
    :caption: 资源宏定义(内核源码/include/linux/ioport.h)
    :linenos:

    #define IORESOURCE_IO		0x00000100	/* PCI/ISA I/O ports */
    #define IORESOURCE_MEM		0x00000200
    #define IORESOURCE_REG		0x00000300	/* Register offsets */
    #define IORESOURCE_IRQ		0x00000400
    #define IORESOURCE_DMA		0x00000800
    #define IORESOURCE_BUS		0x00001000

私有数据
^^^^^^
Linux只提供了六种资源类型，很明显，当我们所需要数据，如某个GPIO，并不包含在上述六种中，为此，诞生了私有数据。在platform_device结构体中，嵌入了device结构体，
该结构体有个变量platform_data，可以用于保存自定义数据。


我们在platform_device结构体中提到过资源，在内核中采用struct resource来表示，如下所示：

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
    };

删除了一些成员变量



平台驱动
~~~~~~

如何注册平台驱动
------

结构体
^^^^^

.. code-block:: c
    :caption: platform_driver结构体(内核源码/include/platform_device.h)
    :linenos:

    struct platform_driver {
        int (*probe)(struct platform_device *);
        int (*remove)(struct platform_device *);
        struct device_driver driver;
        const struct platform_device_id *id_table;
    };

- probe：函数指针类型，指向我们的probe函数，当总线为设备和驱动匹配上之后，会执行驱动的probe函数。我们通常在该函数中，对设备进行一系列的初始化。
- remove:函数指针类型，指向我们的remove函数，当我们移除我们的平台设备时，会调用该函数，该函数实现的操作，通常是probe函数的逆过程。
- driver:
- id_table：表示该驱动能够兼容的设备类型，总线进行匹配时，也会依据该结构体的name成员进行对比。

.. code-block:: c
    :caption: id_table结构体(内核源码/include/linux/mod_devicetable.h)
    :linenos:

    struct platform_device_id {
        char name[PLATFORM_NAME_SIZE];
        kernel_ulong_t driver_data;
    };

我们可以看到，platform_device_id中还有另一个成员driver_data。对于某些设备，他们之间的区别往往可能只是在某个寄存器的地址或者配置不同，我们可以利用成员来区分不同的设备，
这样就可以实现一个驱动可以匹配多个设备的功能。


初始化/移除平台驱动

.. code-block:: c 
    :caption: platform_driver_register函数
    :linenos:

    int platform_driver_register(struct platform_driver *drv);


.. code-block:: c 
    :caption: platform_driver_unregister函数(内核源码/drivers/base/platform.c)
    :linenos:

    void platform_driver_unregister(struct platform_driver *drv);


获取资源API
^^^^^

.. code-block:: c
    :caption: platform_get_resource函数
    :linenos:

    struct resource *platform_get_resource(struct platform_device *dev, unsigned int type, unsigned int num);

.. code-block:: c 
    :caption: platform_get_irq函数
    :linenos:

    int platform_get_irq(struct platform_device *pdev, unsigned int num)




实验
~~~~~~

注册平台设备
------

resource结构体
^^^^

我们定义了两种类型的资源，分别是IORESOURCE_MEM，其起始地址为0x1000,结束地址为0x2000,大小为4096个字节；另一个
则是IORESOURCE_IRQ，它使用的中断编号为1。

.. code-block:: c
    :caption: my_pdev_res结构体数组(文件my_pdev.c) 
    :linenos:

    static struct resource my_pdev_res[] = {
        [0] = {
            .name = "mem",
            .start = 0x1000,
            .end = 0x2000,
            .flags = IORESOURCE_MEM,
            },
        [1] = {
            .name = "irq",
            .start = 0x1,
            .end = 0x1,
            .flags = IORESOURCE_IRQ,
            },
    };



platform_device结构体
^^^^^

在注册平台设备之前，我们还需要实现platform_device结构体。

.. code-block:: c 
    :caption: my_pdev结构体
    :linenos:

    static int my_pdev_id = 0x1D;

    static void my_pdev_release(struct device *dev)
    {
        return;
    }

    static struct platform_device my_pdev = {
        .id = 0,
        .name = "my_pdev",
        .resource = my_pdev_res,
        .num_resources = ARRAY_SIZE(my_pdev_res),
        .dev = {
            .platform_data = &my_pdev_id,
            .release = my_pdev_release,
            },
    };

我们定义了一个名为my_pdev的平台设备。我们注意到我们定义了一个空的my_pdev_release函数，这是因为一旦我们没定义该函数时，移除平台设备时，会提示“
Device 'xxxx' does not have a release() function, it is broken and must be fixed”的错误。此外，我们的私有数据设置为my_pdev_id变量的地址。

注册平台设备
^^^^^

.. code-block:: c 
    :caption: my_pdev_init函数(文件my_pdev.c)
    :linenos:

    static __init int my_pdev_init(void)
    {
        printk("my_pdev module loaded\n");

        platform_device_register(&my_pdev);

        return 0;
    }

    module_init(my_pdev_init);

移除平台设备
^^^^^

.. code-block:: c 
    :caption: my_pdev_exit函数(文件my_pdev.c)
    :linenos:

    static __exit void my_pdev_exit(void)
    {
        printk("my_pdev module unloaded\n");

        platform_device_unregister(&my_pdev);
    }

    module_exit(my_pdev_exit);


注册平台设备
------

platform_device_id结构体
^^^^^

.. code-block:: c 
    :caption: my_pdev_ids结构体(文件my_pdrv.c)
    :linenos:
    static int index0 = 0;
    static int index1 = 1;

    static struct platform_device_id my_pdev_ids[] = {
        {.name = "my_pdev",.driver_data = &index0},
        {.name = "my_test",.driver_data = &index1},
        {}
    };

    MODULE_DEVICE_TABLE(platform, my_pdev_ids);


probe函数
^^^^^

.. code-block:: c 
    :caption: my_pdrv_probe函数(文件my_pdrv.c)
    :linenos:

    static int my_pdrv_probe(struct platform_device *pdev)
    {
        struct resource *mem = NULL;
        int irq;
        struct platform_device_id *id_match = pdev->id_entry;
        int *pdev_id = NULL;
        name = id_match->name;
        index = id_match->driver_data;
        printk("Hello! %s probed!The index is : %d\n", name, *index);

        mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (!mem) {
            printk("Resource not available\n");
            return -1;
        }
        printk("The name : %s, The start : %d, The end : %d\n", mem->name,
            mem->start, mem->end);
        irq = platform_get_irq(pdev, 0);
        printk("The irq : %d\n", irq);

        pdev_id = dev_get_platdata(&pdev->dev);
        printk("The device id : 0x%x\n", *pdev_id);
        return 0;
    }


remove函数
^^^^^

由于我们的驱动比较简单，在probe函数并没有申请什么内存，因此，remove函数也就不需要进行资源的释放。

.. code-block:: c 
    :caption: my_pdrv_remove函数(文件my_pdrv.c)
    :linenos:

    static int my_pdrv_remove(struct platform_device *pdev)
    {
        printk("Hello! %s removed!The index is : %d\n", name, *index);
        return 0;
    }

platform_device结构体
^^^^^

.. code-block:: c 
    :caption: my_pdrv结构体
    :linenos:

    static struct platform_driver my_pdrv = {
        .probe = my_pdrv_probe,
        .remove = my_pdrv_remove,
        .driver = {
            .name = "my_pdev",
            .owner = THIS_MODULE,
            },
        .id_table = my_pdev_ids,
    };

注册平台驱动
^^^^

.. code-block:: c 
    :caption: my_pdrv_init函数
    :linenos:

    static __init int my_pdrv_init(void)
    {
        printk("my_pdrv module loaded\n");

        platform_driver_register(&my_pdrv);

        return 0;
    }

    module_init(my_pdrv_init);

移除平台驱动
^^^

.. code-block:: c 
    :caption: my_pdrv_exit函数
    :linenos:

    static __exit void my_pdrv_exit(void)
    {
        printk("my_pdrv module unloaded\n");

        platform_driver_unregister(&my_pdrv);

    }

    module_exit(my_pdrv_exit);   



Makefile
------

.. code-block:: c 
    :caption: Makefile
    :linenos:

    KERNEL_DIR = /home/wind/ebf_6ull_linux

    obj-m := my_pdev.o my_pdrv.o

    all:modules
    modules clean:
        $(MAKE) -C $(KERNEL_DIR) M=$(shell pwd) $@

实验结果
-----

