.. vim: syntax=rst

PWM子系统
------

PWM子系统用于管理PWM波的输出，与我们之前学习的其他子系统类似,PWM具体实现代码由芯片厂商提供并默认编译进内核，而我们可以使用内核（pwm子系统）提供的一些接口函数来实现具体的功能，例如使用PWM波控制显示屏的背光、控制无源蜂鸣器等等。

pwm子系统功能单一，很少单独使用。PWM子系统的使用也非常简单，我们这
章通过一个极简的PWM子系统驱动来简单认识一下PWM子系统。其中讲解的一些接口函数后
面的复杂驱动可能会用到。

本章配套源码和设备树插件位于“~/embed_linux_tutorial/base_code/pwm_sub_system”目录下。

PWM子系统简介
~~~~~~~~

在i.mx6中pwm子系统使用的是PWM外设。共有8个，有关PWM外设的介绍可以参考imx6ull参考手册的Chapter 38 Pulse Width Modulation
(PWM)章节，这里不再介绍。使用了PWM子系统后和具体硬件相关的内容芯片厂商已经写好了，我们唯一要做的就是在设备树（或者是设备树插件）中声明使用的引脚。

PWM设备结构体
^^^^^^^^

在驱动中使用pwm_device结构体代表一个PWM设备。结构体原型如下所示：

.. code-block:: c 
    :caption: pwm_device结构体
    :linenos:

    struct pwm_device {
    	const char		*label;
    	unsigned long		flags;
    	unsigned int		hwpwm;
    	unsigned int		pwm;
    	struct pwm_chip		*chip;
    	void			*chip_data;
    
    	unsigned int		period; 	---------①
    	unsigned int		duty_cycle;	---------②
    	enum pwm_polarity	polarity;   ---------③
    };

pwm_device结构体中几个重要的参数介绍如下，标号①，设置PWM的周期，这里的单位是纳秒(ns)。例如我们要输出一个1MHz的PWM波，那么period需要设置为1000.。标号②处duty_cycle设置占空比，如果是正常的输出极性，这个参数指定PWM波一个周期内高电平持续时间，单位还是ns
，很明显duty_cycle不能大于period。如果设置非输出反相，则该参数用于指定一个周期内低电平持续时间。标号③处polarity参数用于指定输出极性，既PWM输出是否反相。它是一个枚举类型，如下所示。


.. code-block:: c 
    :caption: pwm_polarity枚举类型
    :linenos:

    enum pwm_polarity {
    	PWM_POLARITY_NORMAL,
    	PWM_POLARITY_INVERSED,
    };

PWM_POLARITY_NORMAL表示正常模式，不反相。PWM_POLARITY_INVERSED表示输出反相。

pwm的申请和释放函数
^^^^^^^^^^^

PWM使用之前要申请，不用时要释放。相关函数如下所示。



.. code-block:: c 
    :caption: pwm的申请和释放函数
    :linenos:

    /*---------------第一组---------------*/
    struct pwm_device *pwm_request(int pwm, const char *label);
    void pwm_free(struct pwm_device *pwm);
    
    /*---------------第二组---------------*/
    struct pwm_device *pwm_get(struct device *dev, const char *con_id)
    void pwm_put(struct pwm_device *pwm)
    
    /*---------------第三组---------------*/
    struct pwm_device *devm_pwm_get(struct device *dev, const char *con_id)
    void devm_pwm_put(struct device *dev, struct pwm_device *pwm)
    
    /*---------------第四组---------------*/
    struct pwm_device *of_pwm_get(struct device_node *np, const char *con_id)
    struct pwm_device *devm_of_pwm_get(struct device *dev, struct device_node *np,
    				   const char *con_id)
    



申请和释放函数很多，共分为四组，介绍如下：

第一组，这是旧的系统使用的pwm申请和释放函数，现在已经弃用，看到之后认识即可。这里不介绍。

第二组，pwm_get，PWM申请函数，pwm_put，pwm释放函数。pwm_get有两个参数。dev参数，从哪个设备获取PWM,内核会在dev设备的设备树节点中根据参数“con_id”查找，判断依据是con_id与设备树节点的"pwm-
names"相同。如果设备中只用了一个PWM则可以将参数con_id设置为NULL，并且在设备树节点中不用设置“pwm-names”属性。获取成功后返回得到的pwm。失败返回NULL。

第三组,这一组函数是对上一组函数的封装，使用方法和第二组相同，优点是当驱动移除时自动注销申请的pwm

第四组，of_pwm_get函数，从指定的设备树节点获取PWM，np参数指定从哪个设备节点获取PWM，参数“con_id”作用和前几组函数一样。返回值是获取得到的PWM，失败则返回NULL。函数devm_of_pwm_get是对of_pwm_get函数的封装，区别是它有三个参数，参数dev指定那个设
备要获取PWM ，其他两个与of_pwm_get函数相同，它的优点是在驱动移除之前自动注销申请的pwm。

pwm配置函数和使能/停用函数
^^^^^^^^^^^^^^^

申请成功后只需使用函数配置pwm的频率和占空比然后使能输出即可在设定的引脚上输出PWM波。函数很简单，如下所示。


.. code-block:: c 
    :caption: pwm配置函数和启动/停用函数
    :linenos:

    int pwm_config(struct pwm_device *pwm, int duty_ns, int period_ns)
    int pwm_set_polarity(struct pwm_device *pwm, enum pwm_polarity polarity)
    int pwm_enable(struct pwm_device *pwm)
    void pwm_disable(struct pwm_device *pwm)

函数pwm_config用于配置PWM的频率和占空比，需要注意的是这里是通过设置PWM一个周期的时间和高电平时间来设置PWM的频率和占空比，单位都是ns。函数int pwm_set_polarity(struct pwm_device \*pwm, enum pwm_polarity
polarity)用于设置PWM极性，需要注意的是如果这里设置PWM为负极性则函数pwm_config中的参数duty_ns设置的是一个周期内低电平时间。

使能和停用函数很简单不再介绍。

pwm输出实验
~~~~~~~

由于PWM子系统很少单独使用，这里仅仅用一个极简的示例驱动程序介绍PWM子系统的使用。我们把RGB灯的红灯引脚复用为PWM3的输出，在驱动程序中通过设置占空比调整红灯亮度，同样也可以使用示波器观察、验证输出是否正确。

示例程序主要包含两部分内容，第一，添加相应的设备树节点（这里使用设备树插件）。第二，编写测试驱动程序。

添加pwm相关设备树插件
^^^^^^^^^^^^

首先简单介绍一下设备树中的PWM相关内容。打开“imx6ull.dtsi”文件，直接搜索“pwm”在文件中找到如下内容。


.. code-block:: c 
    :caption: pwm节点
    :linenos:

    pwm1: pwm@2080000 {
    	compatible = "fsl,imx6ul-pwm", "fsl,imx27-pwm";
    	reg = <0x2080000 0x4000>;
    	interrupts = <GIC_SPI 83 IRQ_TYPE_LEVEL_HIGH>;
    	clocks = <&clks IMX6UL_CLK_PWM1>,
    		 <&clks IMX6UL_CLK_PWM1>;
    	clock-names = "ipg", "per";
    	# = <2>;
    
    pwm2: pwm@2084000 {
    	compatible = "fsl,imx6ul-pwm", "fsl,imx27-pwm";
    	reg = <0x2084000 0x4000>;
    	interrupts = <GIC_SPI 84 IRQ_TYPE_LEVEL_HIGH>;
    	clocks = <&clks IMX6UL_CLK_DUMMY>,
    		 <&clks IMX6UL_CLK_DUMMY>;
    	clock-names = "ipg", "per";
    	#pwm-cells = <2>;
    };

这里就是PWM驱动对应的设备树节点，这是pwm子系统的控制节点，可以看到它设置了imx6ull芯片pwm外设的时钟、中断、寄存器地址等等。这样的节点共有8个分别对应pwm1~pwm8。简单了解即可，我们不会去修改它。

使用pwm 只需要在设备树节点中添加两条属性信息，如下所示


.. code-block:: c 
    :caption: pwm属性信息
    :linenos:

    pwms = <&phandle id period_ns>;
    pwm-names = "name";

pwms属性是必须的，它共有三个属性值“&PWMn”指定使用哪个pwm，共有8个可选，定义在imx6ull.dtsi文件。“id”pwm的id通常设置为0。“period_ns”用于设置周期。单位是ns。

本实验只使用了一个gpio 设备树插件源码如下所示。


.. code-block:: c 
    :caption: 设备树插件
    :linenos:

     / {
         fragment@0 {
              target-path = "/";
             __overlay__ { 
    			/*----------------第一部分-------------*/
    	        red_led_pwm {
    	        	compatible = "red_led_pwm";
    	        	pinctrl-names = "default";
    	        	pinctrl-0 = <&red_led_pwm>;
    
    	        	front {
    					pwm-names = "red_led_pwm3"
    	        		pwms = <&pwm3 0 50000>;
    	        	};
    	        };   
             };
         };
    
         fragment@1 {
             target = <&iomuxc>;
             __overlay__ { 
    			 /*----------------第二部分-------------*/
    	        red_led_pwm: ledsgrp {
    	        	fsl,pins = <
    	        		MX6UL_PAD_GPIO1_IO04__PWM3_OUT 0x1b0b0
    	        	>;
    			};
             };
         };
     };

设备树插件分为两部分，第二部分是添加RGB灯红灯引脚，并把它复用为PWM3的输出。注意，如果之前做过RGB灯的其他实验，要检查下RGB红灯引脚是否被重复使用。第一部分是我们新添加的red_led_pwm节点，red_led_pwm节点包含一个“front”子节点，子节点内定义了pwm属性信息，这里我
们使用PWM3，频率设置为100KHz（周期为50000ns,计算得到频率为100KHz）

驱动程序实现
^^^^^^

驱动程序很简单，使用前面介绍的几个函数即可，具体代码如下：

.. code-block:: c 
    :caption: 注册平台设备
    :linenos:

     static const struct of_device_id of_pwm_leds_match[] = {
    	{.compatible = "red_led_pwm"},
    	{},
    };
    
    static struct platform_driver led_pwm_driver = {
    	.probe		= led_pwm_probe_new,
    	.remove		= led_pwm_remove,
    	.driver		= {
    		.name	= "led_pwm",
    		.of_match_table = of_pwm_leds_match,
    	},
    };
    
    /*
    *驱动初始化函数
    */
    static int __init pwm_leds_platform_driver_init(void)
    {
    	int DriverState;
    	DriverState = platform_driver_register(&led_pwm_driver);
    	return 0;
    }
    
    /*
    *驱动注销函数
    */
    static void __exit pwm_leds_platform_driver_exit(void)
    {
    	printk(KERN_ERR " pwm_leds_exit\n");
    	/*注销平台设备*/
    	platform_driver_unregister(&led_pwm_driver);
    }
    
    module_init(pwm_leds_platform_driver_init);
    module_exit(pwm_leds_platform_driver_exit);
    
    MODULE_LICENSE("GPL");

为简化驱动程序，这里注册了一个平台设备，平台设备与设备节点匹配成功后我们就可以很容易从设备树中获取信息，而不必使用of函数直接从设备树节点中获取，当然也可以尝试其他方法。

我们在.prob函数中申请、设置、使能PWM，具体代码如下：


.. code-block:: c 
    :caption: prob函数
    :linenos:

    static int led_pwm_probe(struct platform_device *pdev)
    {
    	int ret = 0;
    	struct device_node *child; // 保存子节点
    	struct device *dev = &pdev->dev;
    	printk("match success \n");
    
    	/*--------------第一部分-----------------*/
    	child = of_get_next_child(dev->of_node, NULL);
    	if (child)
    	{
    		/*--------------第二部分-----------------*/
    		red_led_pwm = devm_of_pwm_get(dev, child, NULL);
    		if (IS_ERR(red_led_pwm)) 
    		{
    			printk(KERN_ERR" red_led_pwm,get pwm  error!!\n");
    			return -1;
    		}
    	}
    	else
    	{
    		printk(KERN_ERR" red_led_pwm of_get_next_child  error!!\n");
    		return -1;
    	}
    
    
    	
    	/*--------------第三部分-----------------*/
    	pwm_config(red_led_pwm, 1000, 5000);
    	pwm_set_polarity(red_led_pwm, PWM_POLARITY_INVERSED);
    	pwm_enable(red_led_pwm);
    
    	return ret;
    }
    
    static int led_pwm_remove(struct platform_device *pdev)
    {
    	pwm_config(red_led_pwm, 0, 5000);
    	pwm_free(red_led_pwm);
    	return 0;
    }

代码很简单，简单说明如下。第一部分，获取子节点，在设备树插件中，我们把PWM相关信息保存在red_led_pwm的子节点中，所以这里首先获取子节点。第二部分，子节点获取成功后我们使用devm_of_pwm_get函数获取pwm，由于节点内只有一个PWM 这里将最后一个参数直接设置为NULL，这样它将
获取第一个PWM。第三部分，依次调用pwm_config、pwm_set_polarity、pwm_enable函数配置PWM、设置输出极性、使能PWM输出，需要注意的是这里设置的极性为负极性，这样pwm_config函数第二个参数设置的就pwm波的一个周期内低电平事件，数值越大RGB红灯越亮。

下载验证
^^^^

首先编译、加载设备树插件，特别提醒，如果之前添加过RGB灯相关的设备树插件，记得先屏蔽掉，防止RGB灯引脚被重复配置，这会导致错误。编译驱动程序，将.ko文件拷贝到开发板直接使用insmod命令加载驱动，正常情况下可以看到RGB红灯亮度较低，使用示波器也可以看到设定的PWM波（如果不更改例程配置，p
wm频率为100KHz,占空比80

%）。
