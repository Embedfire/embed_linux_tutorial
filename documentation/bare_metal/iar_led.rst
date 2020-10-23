.. vim: syntax=rst

使用IAR集成开发环境点亮LED灯
------------------------------------------------------------------------------------------------------------------

通过上面章节我们知道了如何使用汇编及Linux下的arm gcc工具控制GPIO引脚点亮LED，
在SDK的“ **SDK_2.2_MCIM6ULL_EBF6ULL\boards\evkmcimx6ull\driver_examples** ”目录下存在
大量的基于IAR集成开发环境的官方例程。这章节将使用IAR集成开发环境来点亮LED，
但除本章以外的其他裸机章节不会使用IAR集成开发环境，而是采用linux下的arm gcc工具开发。


官方SDK中提供了大量的开发好的裸机驱动，是作为裸机以及驱动开发的重要参考资料。
对此我们必须要有一定的了解，需要能够快速找到我们需要的内容。
同时IAR集成开发环境借助调试工具（jlink）能够实现强大的调试功能，
在开发阶段它能够帮助我们快速“调通”硬件，程序在IAR上调通后再移植到Linux 的arm gcc并不难。

本章目标：

- 了解IAR对IMX6ULL的裸机开发。



配套源码:

-  路径：**~/embed_linux_tutorial/base_code/bare_metal/IAR_project**
-  野火裸机下载工具download_tool
   （路径：**~/embed_linux_tutorial/base_code/bare_metal/download-tool/download-tool.tar.bz2** ）。




使用IAR点亮LED灯实验
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

本教程假设大家都有一定的单片机基础，这里就不开始从头开始建立工程了，
我们的IAR工程名为"新建工程-固件库版本"，工程结构如下图所示。

.. image:: media/iarled010.png
   :align: center
   :alt: 未找到图片


查看底板原理图
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

打开开发板相对应的原理图来查看硬件连接，具体如下图所示。

.. image:: media/iarled011.png
   :align: center
   :alt: 未找到图片

在前面的章节中已经详细得介绍了如何从原理图以及数据手册中查找GPIO所对应的编号，
这里介绍另一种查找方式，在官方写好的文件中查找，
我们打开"fsl_iomuxc.h"文件（可以打开IAR工程找到该文件也可以在工程目录下直接搜索）。
直接在"fsl_iomuxc.h"文件中搜索上图LED灯所对应的引脚CSI_HSYNC（或CSI_VSYNC）
得到如下图所示的结果（以CSI_HSYNC为例）。

.. image:: media/iarled015.png
   :align: center
   :alt: 未找到图片


从图中不难看出这就是我们要找的引脚，每个宏定义包含着三个信息，
以宏IOMUXC_CSI_HSYNC_I2C2_SCL为例，IOMUXC代表这是一个引脚复用宏定义，
CSI_HSYNC代表原理图上实际的芯片引脚名，I2C2_SCL代表引脚的复用功能。
一个引脚有多个复用功能，本章要把CSI_HSYNC用作GPIO控制LED灯，
所以本实验要选择IOMUXC_CSI_HSYNC_GPIO4_IO20宏定义引脚CSI_HSYNC复用为GPIO4_IO20，
具体怎么使用程序中再详细介绍。


各个LED灯的连接信息及相应引脚的GPIO端口和引脚号如下表所示。


===== ============ ========== ==================
LED灯 原理图的标号 具体引脚名 GPIO端口及引脚编号
===== ============ ========== ==================
R灯   GPIO_4       GPIO1_IO04 GPIO1_IO04
G灯   CSI_HSYNC    CSI_HSYNC  GPIO4_IO20
B灯   CSI_VSYNC    CSI_VSYNC  GPIO4_IO19
===== ============ ========== ==================

软件设计
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

这里讲解核心部分的代码，完整的代码请参考本章配套的工程。

LED相关代码存储在bap_led.c/h，引脚复用功能定义在fsl_iomuxc.h文件，引脚属性（输入输出模式等其他属性）定义在pad_config.h

编程要点
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. 根据引脚号定义GPIO控制相关的宏；

2. 使用IOMUXC外设配置MUX及PAD；

3. 使用GPIO外设配置引脚方向及中断模式；

4. 编写简单测试程序，控制GPIO引脚输出高、低电平。

代码分析
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

LED灯引脚宏定义
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

在编写应用程序的过程中，我们需要考虑更改硬件环境的情况，例如LED灯的控制引脚与当前的不一样，
我们希望程序只做最小的修改即可在新的环境上正常运行。
这个时候一般把硬件相关的部分使用宏来封装，若更改了硬件环境，只修改这些硬件相关的宏即可，
这些定义一般存储在头文件，即本例子中的"bsp_led.h"文件中，具体见代码清单 49‑1。


.. code-block:: c
   :caption: LED控制引脚相关的宏（bsp_led.h文件）
   :linenos:  

   #define RGB_RED_LED_GPIO                 GPIO1
    #define RGB_RED_LED_GPIO_PIN            (4U)
    #define RGB_RED_LED_IOMUXC              IOMUXC_GPIO1_IO04_GPIO1_IO04
   
    #define RGB_GREEN_LED_GPIO              GPIO4
    #define RGB_GREEN_LED_GPIO_PIN          (20U)
    #define RGB_GREEN_LED_IOMUXC            IOMUXC_CSI_HSYNC_GPIO4_IO20
   
    #define RGB_BLUE_LED_GPIO               GPIO4
    #define RGB_BLUE_LED_GPIO_PIN           (19U)
    #define RGB_BLUE_LED_IOMUXC             IOMUXC_CSI_VSYNC_GPIO4_IO19
   
   
以上代码分别把控制三盏LED灯的GPIO端口、GPIO引脚号以及IOMUXC的复用功能根据硬件连接使用宏定义封装起来了。
在实际控制的时候我们就直接用这些宏，以达到应用代码跟硬件无关的效果。

LED GPIO初始化驱动
''''''''''''''''''''''''''

利用上面的宏，我们在bsp_led.c文件中编写LED灯的初始化驱动，具体如下所示。


.. code-block:: c
   :caption: GPIO初始化驱动(bsp_led.c文件)
   :linenos:  

   #include "fsl_iomuxc.h"
   #include "fsl_gpio.h"  
   
   #include "pad_config.h"  
   #include "./led/bsp_led.h"   


    /* 所有引脚均使用同样的PAD配置 */
    #define LED_PAD_CONFIG_DATA  (SRE_0_SLOW_SLEW_RATE| \
                                  DSE_6_R0_6| \
                                  SPEED_2_MEDIUM_100MHz| \
                                  ODE_0_OPEN_DRAIN_DISABLED| \
                                  PKE_0_PULL_KEEPER_DISABLED| \
                                  PUE_0_KEEPER_SELECTED| \
                                  PUS_0_100K_OHM_PULL_DOWN| \
                                  HYS_0_HYSTERESIS_DISABLED)   
        /* 配置说明 : */
        /* 转换速率: 转换速率慢
          驱动强度: R0/6 
          带宽配置 : medium(100MHz)
          开漏配置: 关闭 
          拉/保持器配置: 关闭
          拉/保持器选择: 保持器（上面已关闭，配置无效）
          上拉/下拉选择: 100K欧姆下拉（上面已关闭，配置无效）
          滞回器配置: 关闭 */     
   
    /************************************************
     * 声明
     *****************************************************/
    static void LED_IOMUXC_MUX_Config(void);
    static void LED_IOMUXC_PAD_Config(void);
    static void LED_GPIO_Mode_Config(void);
   

    /**
    * @brief  初始化LED相关IOMUXC的MUX复用配置
    */
    static void LED_IOMUXC_MUX_Config(void)
    {
      /* RGB LED灯，使用同样的IOMUXC MUX配置 */  
      IOMUXC_SetPinMux(RGB_RED_LED_IOMUXC, 0U); 
      IOMUXC_SetPinMux(RGB_BLUE_LED_IOMUXC, 0U);  
      IOMUXC_SetPinMux(RGB_GREEN_LED_IOMUXC, 0U);
    }
   

    /**
    * @brief  初始化LED相关IOMUXC的MUX复用配置
    */
    static void LED_IOMUXC_PAD_Config(void)
    { 
      /* RGB LED灯，使用同样的IOMUXC PAD配置 */ 
      IOMUXC_SetPinConfig(RGB_RED_LED_IOMUXC, LED_PAD_CONFIG_DATA); 
      IOMUXC_SetPinConfig(RGB_GREEN_LED_IOMUXC, LED_PAD_CONFIG_DATA); 
      IOMUXC_SetPinConfig(RGB_BLUE_LED_IOMUXC, LED_PAD_CONFIG_DATA);  
    }
   

     /**
      * @brief  初始化LED相关的GPIO模式
      */
    static void LED_GPIO_Mode_Config(void)
    {     
      /* 定义gpio初始化配置结构体 */
      gpio_pin_config_t led_config;      
      
       /** 核心板的LED灯，GPIO配置 **/       
      led_config.direction = kGPIO_DigitalOutput; //输出模式
      led_config.outputLogic =  1;                //默认高电平    
      led_config.interruptMode = kGPIO_NoIntmode; //不使用中断
      
      /* 使用同样的LED config配置RGB LED灯 */
      GPIO_PinInit(RGB_RED_LED_GPIO,RGB_RED_LED_GPIO_PIN,&led_config);
   GPIO_PinInit(RGB_GREEN_LED_GPIO,RGB_GREEN_LED_GPIO_PIN,&led_config);
     GPIO_PinInit(RGB_BLUE_LED_GPIO,RGB_BLUE_LED_GPIO_PIN,&led_config);
    }
   

    /**
      * @brief  初始化控制LED的IO
      */
    void LED_GPIO_Config(void)
    {
      /* 初始化GPIO复用、属性、模式 */
        LED_IOMUXC_MUX_Config();
        LED_IOMUXC_PAD_Config();
    LED_GPIO_Mode_Config();
    }



整个驱动文件主要是把初始化LED的内容分成了MUX配置函数、PAD属性函数以及GPIO模式配
置函数几部分，最后再把它们封装进了一个函数方便调用，另外还增加了对底板RGB LED灯的
初始化，该代码的各个部分说明如下：

- 第1-5行，包含了头文件fsl_iomuxc.h、fsl_gpio.h、pad_config.h及bsp_led.h。
  其中的fsl_iomuxc.h和fsl_gpio.h是NXP固件库文件，它们分别包含了控制IOMUXC和GPIO外设的类型定义和函数声明，
  我们在下面的代码将会使用这些库文件提供的函数。
  pad_config.h和bsp_led.h文件都是我们自己创建的，
  其中bsp_led.h文件中定义了各个LED控制引脚及操作宏，
  pad_config.h文件主要包含使用IOMUXC外设配置PAD寄存器的引脚属性时使用的宏，具体如下


.. code-block:: c
   :caption: LED控制引脚相关的宏（bsp_led.h文件）
   :linenos:  

   #include "fsl_common.h"
   
    /* SRE 压摆率选择 */
    #define SRE_0_SLOW_SLEW_RATE    IOMUXC_SW_PAD_CTL_PAD_SRE(0)
    #define SRE_1_FAST_SLEW_RATE    IOMUXC_SW_PAD_CTL_PAD_SRE(1)
   
    /* 驱动能力配置，配置阻值的大小 */
    #define DSE_0_OUTPUT_DRIVER_DISABLED  IOMUXC_SW_PAD_CTL_PAD_DSE(0)
    /* R0 260 Ohm @ 3.3V, 150Ohm@1.8V, 240 Ohm for DDR */
    #define DSE_1_R0_1               IOMUXC_SW_PAD_CTL_PAD_DSE(1) 
    /* R0/2 */
    #define DSE_2_R0_2               IOMUXC_SW_PAD_CTL_PAD_DSE(2)
    /* R0/3 */
    #define DSE_3_R0_3               IOMUXC_SW_PAD_CTL_PAD_DSE(3)
    /* R0/4 */
    #define DSE_4_R0_4               IOMUXC_SW_PAD_CTL_PAD_DSE(4)
    /* R0/5 */
    #define DSE_5_R0_5               IOMUXC_SW_PAD_CTL_PAD_DSE(5)
    /* R0/6 */
    #define DSE_6_R0_6               IOMUXC_SW_PAD_CTL_PAD_DSE(6)
    /* R0/7 */
    #define DSE_7_R0_7               IOMUXC_SW_PAD_CTL_PAD_DSE(7)
   
    /* SPEED 带宽配置 */
    #define SPEED_0_LOW_50MHz            IOMUXC_SW_PAD_CTL_PAD_SPEED(0)
    #define SPEED_1_MEDIUM_100MHz        IOMUXC_SW_PAD_CTL_PAD_SPEED(1)
    #define SPEED_2_MEDIUM_100MHz        IOMUXC_SW_PAD_CTL_PAD_SPEED(2)
    #define SPEED_3_MAX_200MHz           IOMUXC_SW_PAD_CTL_PAD_SPEED(3)
   
    /* ODE 是否使用开漏模式 */
    #define ODE_0_OPEN_DRAIN_DISABLED  IOMUXC_SW_PAD_CTL_PAD_ODE(0)     
    #define ODE_1_OPEN_DRAIN_ENABLED   IOMUXC_SW_PAD_CTL_PAD_ODE(1)     
   
    /* PKE 是否使能保持器或上下拉功能 */
    #define PKE_0_PULL_KEEPER_DISABLED   IOMUXC_SW_PAD_CTL_PAD_PKE(0)      
    #define PKE_1_PULL_KEEPER_ENABLED    IOMUXC_SW_PAD_CTL_PAD_PKE(1)      
   
    /* PUE 选择使用保持器还是上下拉 */
    #define PUE_0_KEEPER_SELECTED        IOMUXC_SW_PAD_CTL_PAD_PUE(0)   
    #define PUE_1_PULL_SELECTED          IOMUXC_SW_PAD_CTL_PAD_PUE(1)   
   
    /* PUS 上下拉配置 */
    #define PUS_0_100K_OHM_PULL_DOWN     IOMUXC_SW_PAD_CTL_PAD_PUS(0)     
    #define PUS_1_47K_OHM_PULL_UP        IOMUXC_SW_PAD_CTL_PAD_PUS(1)   
    #define PUS_2_100K_OHM_PULL_UP       IOMUXC_SW_PAD_CTL_PAD_PUS(2)   
    #define PUS_3_22K_OHM_PULL_UP        IOMUXC_SW_PAD_CTL_PAD_PUS(3)


NXP固件库本身并没有提供这些内容，因此我们为了方便使用而把它独立编写在这个自建的pad_config.h文件了，
在以后对GPIO引脚属性配置时，可以用同样的方式使用这个文件。接下来让我们回到最上面的代码上。


- 第9-16行，它利用pad_config.h文件，定义了一个宏LED_PAD_CONFIG_DATA，这将会在下面代码中使用，
  功能是设定LED引脚的PAD属性配置。由于这4个LED灯的PAD属性配置是完全一样的，所以在此处定义成宏简化代码。
  另外，代码中展示的并不是控制LED灯的唯一配置，如转换速率、驱动强度等也可以使用其它模式，
  都能正常地控制LED灯，感兴趣可以自己修改代码并测试。


- 第38-44行，此处定义了函数LED_IOMUXC_MUX_Config专门用于配置LED灯引脚的MUX复用模式。
  在其内部，每行代码都是直接调用库函数IOMUXC_SetPinMux进行MUX配置。
  由于我们在bsp_led.h文件中用宏定义好了IOMUXC要配置的复用功能，都是作为GPIO功能使用，
  所以在调用这个库函数时，直接用宏IOMUXC_GPIO1_IO04_GPIO1_IO04、IOMUXC_CSI_HSYNC_GPIO4_IO20、
  以及IOMUXC_CSI_VSYNC_GPIO4_IO19作为第一个参数即可。驱动LED灯时，不需要读取回引脚的电平值，
  所以不需要开启SION功能，所以第二个参数被设置为0，当然，开启SION功能也是可以驱动LED灯的。


- 第50-56行，此处定义了函数LED_IOMUXC_PAD_Config专门用于设定LED灯引脚的PAD属性配置。
  在其内部，每行代码都是直接调用库函数IOMUXC_SetPinConfig进行PAD属性配置。类似地，
  在调用库函数时第一个参数用bsp_led.h文件中定义的宏来指定要设置的引脚号；
  第二个参数则直接都使用第2部分中定义的宏LED_PAD_CONFIG_DATA，
  每个控制LED灯的引脚都采用同样的PAD属性配置，可自行修改该宏的值来尝试不同的配置来进行试验。


- 第62-76行，定义了函数LED_GPIO_Mode_Config专门用于设定LED灯引脚的GPIO模式。
  在函数的内部，先是使用库文件中的gpio_pin_config_t类型定义了一个变量led_config，
  它包含了初始化GPIO外设时要指定的方向、默认电平以及中断模式。
  接着，对变量led_config进行赋值，本配置参数为输出模式、默认高电平以及不使用中断。
  赋值完成后使用同一个led_config变量调用库函数GPIO_PinInit对不同的GPIO端口及引脚进行初始化，
  即所有控制LED灯的引脚都采用同样的GPIO配置。


- 第82-88行，这部分代码定义了LED_GPIO_Config函数，它实际上是对上面函数的封装，
  目的是在应用程序中调用本函数就完成LED所有内容的初始化。


特别地，在代码LED初始化函数中并没有设置GPIO的时钟，
原因是因为在GPIO_PinInit函数加入GPIO时钟的开启控制操作具体如下所示。

.. code-block:: c
   :caption: NXP固件库中fls_gpio.c文件中的GPIO_PinInit函数
   :linenos:  

   void GPIO_PinInit(GPIO_Type *base, uint32_t pin,
                     const gpio_pin_config_t *Config)
   {
     
   #if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) &&
       FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
       /* 使能GPIO时钟 */
       CLOCK_EnableClock(s_gpioClock[GPIO_GetInstance(base)]);
   #endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
       
       /* 对相应引脚IMR寄存器的控制位清零，先关闭中断 */
       base->IMR &= ~(1U << pin);
       /* 配置GPIO引脚的方向 */
       if (Config->direction == kGPIO_DigitalInput) {
       /* 输入模式 */
       base->GDIR &= ~(1U << pin);
       } else {
           /* 输出模式 */
           /* 先对DR寄存器赋值默认电平 */
           GPIO_PinWrite(base, pin, Config->outputLogic);
           /* 配置为输出模式 */
           base->GDIR |= (1U << pin);
       }
       /* 配置GPIO引脚的中断模式 */
       GPIO_SetPinInterruptConfig(base, pin, Config->interruptMode);
   }

- 第5-9行，增加了对库函数CLOCK_EnableClock的调用，调用时根据函数输入参数base进行配置，
  而使用时，我们常常把base参数赋值为GPIO1、GPIO2等值，
  即CLOCK_EnableClock函数会根据实际的需要初始化GPIO1、GPIO2等端口的时时钟。
 
- 第12-26行，根据Config参数初始化GPIO的工作模式。


LDE GPIO初始化驱动总结
''''''''''''''''''''''''

下面总结一下我们编写的LED灯驱动：在bsp_led.h文件中定义好具体的硬件引脚及控制亮灭的宏；
在bsp_led.c文件中定义好LED_IOMUXC_MUX_Config、LED_IOMUXC_PAD_Config及LED_GPIO_Mode_Config函数，
这些函数完成IOMUXC外设的MUX复用功能和引脚PAD属性的配置，完成了GPIO外设及相应时钟的初始化。
最后还把这几部分的初始化封装到LED_GPIO_Config函数中。

在后面的LED灯应用中，我们只需要调用LED_GPIO_Config函数即可完成所有LED灯引脚的初始化，然后直接使用宏控制LED灯即可。

main文件
''''''''''''''''''''''''''''''''''''''''''

编写完LED灯的控制函数后，就可以在main函数中测试了，具体如下。


.. code-block:: c
   :caption: 控制LED灯（main文件）
   :linenos:  

   
   #include "fsl_debug_console.h"
   
   #include "board.h"
   #include "pin_mux.h"
   #include "clock_config.h"
   #include "./led/bsp_led.h"   
   
    
   /*简单延时函数*/
   void delay(uint32_t count)
   {
      volatile uint32_t i = 0;
      for (i = 0; i < count; ++i)
      {
         __asm("NOP"); /* 调用nop空指令 */
      }
   }
   
   /**
    * @brief  主函数
    * @param  无
    * @retval 无
    */
   int main(void)
   {
   
      /* 初始化开发板引脚 */
      BOARD_InitPins();
      /* 初始化开发板时钟 */
      BOARD_BootClockRUN();
      /* 初始化调试串口 */
      BOARD_InitDebugConsole();
   
    
      /* 打印系统时钟 */
      PRINTF("\r\n");
      PRINTF("*****欢迎使用野火EBF6UL/6ULL开发板*****\r\n");
      PRINTF("CPU:         %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
      PRINTF("AHB:         %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
      PRINTF("MMDC:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_MmdcClk));
      PRINTF("SYSPLL:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
      PRINTF("SYSPLLPFD0:%d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
      PRINTF("SYSPLLPFD1:  %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
      PRINTF("SYSPLLPFD2:  %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
      PRINTF("SYSPLLPFD3:  %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));  
      /* 在这里添加你的代码^_^. */

     
      /* 初始化LED引脚 */
      LED_GPIO_Config();  
   
      while(1)
      {
         RGB_RED_LED_ON
         delay(0xFFFFF);
         RGB_RED_LED_OFF

         RGB_GREEN_LED_ON
         delay(0xFFFFF);
         RGB_GREEN_LED_OFF

         RGB_BLUE_LED_ON 
         delay(0xFFFFF);
         RGB_BLUE_LED_OFF
      }     
   }


- 第2-7行，包含了几个头文件，各个文件的作用如下：

  **fsl_debug_console.h** ，这是固件库utilities部分提供的调试工具，在本代码第4部分
  中使用的PRINTF函数就是由它提供的，这是我们调试时最常用的工具，把一些信息通过串口打印
  到电脑上位机查看，其用法与C语言标准的printf函数一样。

  **board.h、pin_mux.h及clock_config.h** ，它包含固件库demo中提供的一些板级
  基础配置函数，我们的例程也是直接沿用demo的这些配置，如下面的
  中的BOARD_ConfigMPU、BOARD_InitPins、BOARD_BootClockRUN及BOARD_InitDebugConsole函数。
  在board.h文件中还包含了NXP固件库最基础的fsl_common.h文件，所以有了这个文件我们就不
  用在main文件中再增加一个"#include "fsl_common.h""语句了。

  **bsp_led.h** ，包含了我们控制LED灯相关的函数及宏。


- 第11-18行，定义了一个delay函数用于简单的延时，它的实现非常简单，
  就是在一个for循环内调用CPU的空操作指令，
  调用形式是"__asm("NOP")"。对于这样的函数我们很难直接根据它的输入参数算出具体的延时时间，
  此处我们只是简单地凭感觉使用，也不要求它有精确的延时，
  在后面需要精确延时的地方，会使用其它形式的延时操作代替。

  另外，由于这个函数会被编译器不同程度地优化，所以在我们不同版本的工程中其延时时间也是不一样的。
  例如按照我们的工程模板配置flexspi_nor_release版本的程序优化等级为3级，其余的均为1级，
  所以在使用同样的输入参数时，flexspi_nor_release版本的这个delay函数延时时间明显要更短，
  导致后面使用了本函数延时的流水灯切换时间更短。

- 第29-33行，主要是从官方demo移植过来的基础初始化组件，感兴趣可以在工程中直接查看其源码，
  各个函数的功能简单说明如下：

  **BOARD_InitPins函数** ，该函数在pin_mux.c文件中定义。在官方demo中，整个板子的
  所有引脚与IOMUXC相关的内容都放置在这个函数内，如LED、按键、串口等引脚的IOMUXC配置。按我们程
  序的编写风格，每个外设的初始化配置都放置在独立的文件中，如LED的放在bsp_led.c文件，
  或以后的按键配置放在bsp_key.c文件。
  此处仍调用BOARD_InitPins函数主要是保留了官方对调试串口引脚IOMUXC部分的初始化，
  要使用下面代码的PRINTF函数，必须调用此函数。

  **BOARD_BootClockRUN函数** ，该函数对整个芯片系统的时钟进行了初始化配置，
  具体的配置结果可以从后面的PRINTF函数打印到电脑串口调试助手的信息查看到。

  **BOARD_InitDebugConsole函数** ，这部分初始化了调试用的串口外设，
  它如同我们初始化LED灯时的GPIO外设部分。因此，要使用下面的PRINTF函数，也必须调用此函数。

- 第37-46行，通过串口打印了芯片目前运行时各个时钟的状态。这部分并不是本工程必须的，
  只是我们延续前面工程模板的内容，此处保留也是方便我们调试查看各个时钟的状态。

- 第51行，调用了我们前面编写的LED_GPIO_Config函数，这个函数包含了相关引脚的IOMUXC及GPIO外设的初始化，
  调用后我们就可以控制LED灯了。

- 第53-66行，使用CORE_BOARD_LED_ON/OFF、RGB_RED_LED_ON/OFF等宏直接控制LED灯的亮灭，
  在这部分代码中对于RGB灯是单个LED灯控制的宏，这些宏定义在bsp_led.h中。

以上，就是一个使用i.MX6U标准软件库开发应用的流程。

下载验证
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

IAR版本工程提供了两个版本(Debug和Release)，Release版本下载需要借助SD以及烧录工具WinHex.exe。
本章目的是带领大家熟悉SDK库，所以不使用Release版本，直接使用Debug版本使用JLink调试。

硬件需求：Jlink ,JTAG转接板（或用杜邦线链接）


开发板Jtag接口如下所示。

.. image:: media/iarled016.png
   :align: center
   :alt: 未找到图片


程序版本选择Debug版本如下所示。


.. image:: media/iarled017.png
   :align: center
   :alt: 未找到图片


正确连接开发板、jlink，点击Debug and download 选项即可。正常情况下可以看到RGB灯交替闪烁。

.. |iarled002| image:: media/iarled002.png
   :width: 4.97854in
   :height: 5.20768in
.. |iarled003| image:: media/iarled003.png
   :width: 3.9995in
   :height: 3.73912in
.. |iarled004| image:: media/iarled004.png
   :width: 2.41276in
   :height: 1.49996in
.. |iarled005| image:: media/iarled005.png
   :width: 3.97833in
   :height: 2.67361in
.. |iarled006| image:: media/iarled006.png
   :width: 5.76806in
   :height: 5.37639in
.. |iarled007| image:: media/iarled007.png
   :width: 5.76806in
   :height: 6.67222in
.. |iarled008| image:: media/iarled008.png
   :width: 5.76806in
   :height: 2.23056in
.. |iarled009| image:: media/iarled009.png
   :width: 5.76806in
   :height: 2.20208in
.. |iarled010| image:: media/iarled010.png
   :width: 4.95771in
   :height: 2.9163in
.. |iarled011| image:: media/iarled011.png
   :width: 5.76806in
   :height: 1.96597in
.. |iarled012| image:: media/iarled012.png
   :width: 5.76806in
   :height: 4.15833in
.. |iarled013| image:: media/iarled013.png
   :width: 5.76806in
   :height: 1.91875in
.. |iarled014| image:: media/iarled014.png
   :width: 5.76806in
   :height: 5.81875in
.. |iarled015| image:: media/iarled015.png
   :width: 5.76806in
   :height: 2.22361in
.. |iarled016| image:: media/iarled016.png
   :width: 4.03075in
   :height: 2.48927in
.. |iarled017| image:: media/iarled017.png
   :width: 4.31196in
   :height: 2.33304in
