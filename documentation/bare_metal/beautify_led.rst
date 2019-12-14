.. vim: syntax=rst

完善LED程序
-------

本章节内容处于开发笔记状态，还待整理至最终版的教程。

本章节内容处于开发笔记状态，还待整理至最终版的教程。

在上两章我们分别使用汇编语言和C语言实现了点亮LED灯。仔细分析代码不难发现我们仅仅操作了一个GPIO就需要自己查找、定义那么多寄存器。这样做的缺点很明显，易错、费时、代码可读性差。NXP官方SDK中已经将所有的寄存器以及所有可用引脚的复用功能定义好了，本章将简单介绍这些内容并把它们添加到我们的程序
。

本章主要内容：

-  添加官方寄存器定义文件。

-  添加官方引脚复用以及引脚属性定义文件。

-  使用官方定义的寄存器、引脚设置函数实现RGB灯程序。

官方库文件介绍
~~~~~~~

本章移植的内容主要包括两部分。第一部分，移植官方寄存器定义文件。第二，移植引脚复用以及引脚属性定义文件。

寄存器定义文件
^^^^^^^

在官方SDK的“SDK_2.2_MCIM6ULL_EBF6ULL\devices\MCIMX6Y2”目录下，头文件“MCIMX6Y2.h”保存了i.MX 6U芯片几乎所有的寄存器定义以及中断编号的定义，本章我们只关心寄存器的定义，有关中断以及中断编号将会在中断章节详细介绍。部分寄存器定义如代码清单
51‑1所示。

代码清单 51‑1寄存器定义

1 typedef struct {

2 \__IO uint32_t DR; /**< GPIO data register, offset: 0x0 \*/

3 \__IO uint32_t GDIR; /**< GPIO direction register, offset: 0x4 \*/

4 \__I uint32_t PSR; /**< GPIO pad status register, offset: 0x8 \*/

5 \__IO uint32_t ICR1; /**< GPIO interrupt configuration register1,*/

6 \__IO uint32_t ICR2; /**< GPIO interrupt configuration register2, \*/

7 \__IO uint32_t IMR; /**< GPIO interrupt mask register, offset: 0x14 \*/

8 \__IO uint32_t ISR; /**< GPIO interrupt status register, offset: 0x18 \*/

9 \__IO uint32_t EDGE_SEL;/**< GPIO edge select register, offset: 0x1C \*/

10 } GPIO_Type;

11

12 /以下代码省略8*/

13 /*\* Peripheral GPIO1 base address \*/

14 #define GPIO1_BASE (0x209C000u)

15 /*\* Peripheral GPIO1 base pointer \*/

16 #define GPIO1 ((GPIO_Type \*)GPIO1_BASE)

这里只列GPIO1相关寄存器的部分截图。其他寄存器定义与此类似。添加这些定义之后我们就可以直接使用“GPIO1->DR”语句操作GPIO1的DR寄存器。操作方法与STM32非常相似。

引脚复用和引脚属性定义文件
^^^^^^^^^^^^^

使用每一个引脚之前我们都要选择引脚的复用功能以及引脚的pad属性。在官方SDK中定义了所有可用引脚以及这些引脚的所有复用功能，我们需要哪种复用功能只需要选择即可，并且官方SDK中提供了初始化函数。如代码清单 51‑2所示：

代码清单 51‑2引脚复用与PAD属性定义（fsl_iomuxc.h）

1 /第一部分/

2 #define IOMUXC_GPIO1_IO00_I2C2_SCL

3 \\ 0x020E005CU, 0x0U, 0x020E05ACU, 0x1U, 0x020E02E8U

4 #define IOMUXC_GPIO1_IO00_GPT1_CAPTURE1L

5 \\ 0x020E005CU, 0x1U, 0x020E058CU, 0x0U, 0x020E02E8U

6 #define IOMUXC_GPIO1_IO00_ANATOP_OTG1_IDL

7 \\ 0x020E005CU, 0x2U, 0x020E04B8U, 0x0U, 0x020E02E8U

8 #define IOMUXC_GPIO1_IO00_ENET1_REF_CLK1L

9 \\ 0x020E005CU, 0x3U, 0x020E0574U, 0x0U, 0x020E02E8U

10 #define IOMUXC_GPIO1_IO00_MQS_RIGHTL

11 \\ 0x020E005CU, 0x4U, 0x00000000U, 0x0U, 0x020E02E8U

12 #define IOMUXC_GPIO1_IO00_GPIO1_IO00L

13 \\ 0x020E005CU, 0x5U, 0x00000000U, 0x0U, 0x020E02E8U

14 #define IOMUXC\_GPIO1_IO00_ENET1_1588_EVENT0_INL

15 \\ 0x020E005CU, 0x6U, 0x00000000U, 0x0U, 0x020E02E8U

16 #define IOMUXC_GPIO1_IO00_SRC_SYSTEM_RESETL

17 \\ 0x020E005CU, 0x7U, 0x00000000U, 0x0U, 0x020E02E8U

18 #define IOMUXC_GPIO1_IO00_WDOG3_WDOG_BL

19 \\ 0x020E005CU, 0x8U, 0x00000000U, 0x0U, 0x020E02E8U

20 #define IOMUXC_GPIO1_IO01_I2C2_SDAL

21 \\ 0x020E0060U, 0x0U, 0x020E05B0U, 0x1U, 0x020E02ECU

22 #define IOMUXC_GPIO1_IO01_GPT1_COMPARE1L

23 \\ 0x020E0060U, 0x1U, 0x00000000U, 0x0U, 0x020E02ECU

24 #define IOMUXC_GPIO1_IO01_USB_OTG1_OCL

25 \\ 0x020E0060U, 0x2U, 0x020E0664U, 0x0U, 0x020E02ECU

26

27 /第二部分/

28 static inline void IOMUXC_SetPinMux(uint32_t muxRegister,

29 uint32_t muxMode,

30 uint32_t inputRegister,

31 uint32_t inputDaisy,

32 uint32_t configRegister,

33 uint32_t inputOnfield)

34 {

35 \*((volatile uint32_t \*)muxRegister) =

36 IOMUXC_SW_MUX_CTL_PAD_MUX_MODE(muxMode) \|\\

37 IOMUXC_SW_MUX_CTL_PAD_SION(inputOnfield);

38

39 if (inputRegister)

40 {

41 \*((volatile uint32_t \*)inputRegister) = \\

42 IOMUXC_SELECT_INPUT_DAISY(inputDaisy);

43 }

44 }

45

46

47 /第三部分/

48 static inline void IOMUXC_SetPinConfig(uint32_t muxRegister,

49 uint32_t muxMode,

50 uint32_t inputRegister,

51 uint32_t inputDaisy,

52 uint32_t configRegister,

53 uint32_t configValue)

54 {

55 if (configRegister)

56 {

57 \*((volatile uint32_t \*)configRegister) = configValue;

58 }

59 }

这里只截取了一小部分代码，结合代码各部分说明如下：

-  第一部分，定义引脚的复用功能。这里只列出了“GPIO1_IO00”引脚的复用功能，其他引脚类似。每个引脚对应多个宏定义代表引脚的不同的复用功能，以宏“IOMUXC_GPIO1_IO00_I2C2_SCL”为例，它表示“GPIO1_IO00”引脚复用为“I2C2”的“SCL”引脚。这些宏定义将会用作
  第二部分和第三部分的函数入口参数。

-  第二部分，引脚复用功能设置函数。函数“IOMUXC_SetPinMux”拥有6个入口参数，但是前五个是通过第一部分的宏定义自动完成设置的。而第6个入口参数“inputOnfiled”用于设置是否开启读回引脚电平功能。

-  第三部分，引脚PAD属性设置函数。与第二部分相同，函数共有6个入口参数，其中前五个是通过第一部分的宏定义自动完成设置的。而第6个参数用于设置PAD属性，根据之前讲解每个引脚拥有一个32位PAD属性寄存器。第六个参数就是设置要填入PAD属性寄存器的值。稍后我们将通过宏定义实现PAD属性设置。

软件设计
~~~~

宏定义实现PAD属性设置
^^^^^^^^^^^^

通常情况下一个引脚要设置8中PAD属性，而这些属性只能通过数字指定。为简化PAD属性设置我们编写了一个PAD属性配置文件“pad_config.h”，这里使用宏定义了引脚可选的PAD属性值，并且通过宏定义的名字很容易知道宏代表的属性值。如代码清单 51‑3所示。

代码清单 51‑3PAD属性设置宏定义(pad_config.h)

1 /第一部分/

2 /\* SPEED 带宽配置 \*/

3 #define SPEED_0_LOW_50MHz IOMUXC_SW_PAD_CTL_PAD_SPEED(0)

4 #define SPEED_1_MEDIUM_100MHz IOMUXC_SW_PAD_CTL_PAD_SPEED(1)

5 #define SPEED_2_MEDIUM_100MHz IOMUXC_SW_PAD_CTL_PAD_SPEED(2)

6 #define SPEED_3_MAX_200MHz IOMUXC_SW_PAD_CTL_PAD_SPEED(3)

7

8 /第二部分/

9 /\* PUE 选择使用保持器还是上下拉 \*/

10 #define PUE_0_KEEPER_SELECTED IOMUXC_SW_PAD_CTL_PAD_PUE(0)

11 #define PUE_1_PULL_SELECTED IOMUXC_SW_PAD_CTL_PAD_PUE(1)

12

13 /第三部分/

14 /\* PUS 上下拉配置 \*/

15 #define PUS_0_100K_OHM_PULL_DOWN IOMUXC_SW_PAD_CTL_PAD_PUS(0)

16 #define PUS_1_47K_OHM_PULL_UP IOMUXC_SW_PAD_CTL_PAD_PUS(1)

17 #define PUS_2_100K_OHM_PULL_UP IOMUXC_SW_PAD_CTL_PAD_PUS(2)

18 #define PUS_3_22K_OHM_PULL_UP IOMUXC_SW_PAD_CTL_PAD_PUS(3)

这里只列出了文件“pad_config.h”部分代码，其他部分类似，结合代码各部分简单说明如下：

-  第一部分，定义引脚带宽。从宏定义名可知带宽可设置为50M、100M、200M。

-  第二部分，定义引脚使用上下拉还是保持器。

-  第三部分，定义引脚的上下拉强度。当引脚设置为上下拉时，这些选项用于设置上下拉电阻大小。

RGB灯代码实现
^^^^^^^^

与手动定义寄存器类似，这里使用官方SDK定义的寄存器并使用SDK提供的基本函数实现RGB灯功能，代码如代码清单 51‑4所示。

代码清单 51‑4RGB灯实现代码

1 /第一部分/

2 #include "MCIMX6Y2.h"

3 #include "fsl_iomuxc.h"

4 #include "pad_config.h"

5

6 /第二部分/

7 /*LED GPIO端口、引脚号及IOMUXC复用宏定义*/

8 #define RGB_RED_LED_GPIO GPIO1

9 #define RGB_RED_LED_GPIO_PIN (4U)

10 #define RGB_RED_LED_IOMUXC IOMUXC_GPIO1_IO04_GPIO1_IO04

11

12 #define RGB_GREEN_LED_GPIO GPIO4

13 #define RGB_GREEN_LED_GPIO_PIN (20U)

14 #define RGB_GREEN_LED_IOMUXC IOMUXC_CSI_HSYNC_GPIO4_IO20

15

16 #define RGB_BLUE_LED_GPIO GPIO4

17 #define RGB_BLUE_LED_GPIO_PIN (19U)

18 #define RGB_BLUE_LED_IOMUXC IOMUXC_CSI_VSYNC_GPIO4_IO19

19

20

21 /第三部分/

22 /\* 所有引脚均使用同样的PAD配置 \*/

23 #define LED_PAD_CONFIG_DATA (SRE_0_SLOW_SLEW_RATE\| \\

24 DSE_6_R0_6\| \\

25 SPEED_2_MEDIUM_100MHz\| \\

26 ODE_0_OPEN_DRAIN_DISABLED\| \\

27 PKE_0_PULL_KEEPER_DISABLED\| \\

28 PUE_0_KEEPER_SELECTED\| \\

29 PUS_0_100K_OHM_PULL_DOWN\| \\

30 HYS_0_HYSTERESIS_DISABLED)

31 /\* 配置说明 : \*/

32 /\* 转换速率: 转换速率慢

33 驱动强度: R0/6

34 带宽配置 : medium(100MHz)

35 开漏配置: 关闭

36 拉/保持器配置: 关闭

37 拉/保持器选择: 保持器（上面已关闭，配置无效）

38 上拉/下拉选择: 100K欧姆下拉（上面已关闭，配置无效）

39 滞回器配置: 关闭 \*/

40

41 /第四部分/

42 /*简单延时函数*/

43 void delay(uint32_t count)

44 {

45 volatile uint32_t i = 0;

46 for (i = 0; i < count; ++i)

47 {

48 \__asm("NOP"); /\* 调用nop空指令 \*/

49 }

50 }

51

52

53 int main()

54 {

55 /第五部分/

56 CCM_CCGR1_CG13(0x3);//开启GPIO1的时钟

57 CCM_CCGR3_CG6(0x3); //开启GPIO4的时钟

58

59 /第六部分/

60 /*设置 红灯 引脚的复用功能以及PAD属性*/

61 IOMUXC_SetPinMux(RGB_RED_LED_IOMUXC,0);

62 IOMUXC_SetPinConfig(RGB_RED_LED_IOMUXC, LED_PAD_CONFIG_DATA);

63

64 /*设置 绿灯 引脚的复用功能以及PAD属性*/

65 IOMUXC_SetPinMux(RGB_GREEN_LED_IOMUXC,0);

66 IOMUXC_SetPinConfig(RGB_GREEN_LED_IOMUXC, LED_PAD_CONFIG_DATA);

67

68 /*设置 蓝灯 引脚的复用功能以及PAD属性*/

69 IOMUXC_SetPinMux(RGB_BLUE_LED_IOMUXC,0);

70 IOMUXC_SetPinConfig(RGB_BLUE_LED_IOMUXC, LED_PAD_CONFIG_DATA);

71

72 /第七部分/

73 GPIO1->GDIR \|= (1<<4); //设置GPIO1_04为输出模式

74 GPIO1->DR \|= (1<<4); //设置GPIO1_04输出电平为高电平

75

76 GPIO4->GDIR \|= (1<<20); //设置GPIO4_20为输出模式

77 GPIO4->DR \|= (1<<20); //设置GPIO4_20输出电平为高电平

78

79 GPIO4->GDIR \|= (1<<19); //设置GPIO4_19为输出模式

80 GPIO4->DR \|= (1<<19); //设置GPIO4_19输出电平为高电平

81

82 /第八部分/

83 while(1)

84 {

85 GPIO1->DR &= ~(1<<4); //红灯亮

86 delay(0xFFFFF);

87 GPIO1->DR \|= (1<<4); //红灯灭

88

89 GPIO4->DR &= ~(1<<20); //绿灯亮

90 delay(0xFFFFF);

91 GPIO4->DR \|= (1<<20); //绿灯灭

92

93 GPIO4->DR &= ~(1<<19); //蓝灯亮

94 delay(0xFFFFF);

95 GPIO4->DR \|= (1<<19); //蓝灯灭

96 }

97 return 0;

98 }

代码很容易理解，这里只做简单的说明。

-  第一部分，添加头文件，文件“MCIMX6Y2.h”和“fsl_iomuxc.h”莱斯SDK。文件“pad_config.h”是自己编写的文件，在其他工程中可直接使用。

-  第二部分，定义LED灯相关引脚以及复用功能。

-  第三部分，定义引脚的PAD属性。PAD属性宏定义保存在“pad_config.h”文件中，这里使用“|”运算符将所有属性设置“合并”在一起，后面将作为函数参数。

-  第四部分，简单的延时函数。

-  第五部分，开启GPIO时钟。

-  第六部分，设置引脚的复用功能以及引脚PAD属性。

-  第七部分，设置GPIO为输出并设置初始电平为高电平。

-  第八部分，在while(1)中依次点亮红灯、绿灯和蓝灯。

编译下载
^^^^

打开本章配套例程，在 文件夹下执行make命令，makefile工具便会自动完成程序的编译、链接、格式转换等工作。正常情况下我们可以在当前目录看到生成的一些中间文件以及我们期待的.bin文件。

在46.2.5 烧写到SD卡章节我们详细讲解了如何将二进制文件烧写到SD卡（烧写工具自动实现为二进制文件添加头）。这里再次说明下载步骤。

-  将一张空SD卡（烧写一定会破坏SD卡中原有数据！！！烧写前请保存好SD卡中的数据），接入电脑后在虚拟机的右下角状态栏找到对应的SD卡。将其链接到虚拟机。

-  进入烧写工具目录，执行“./mkimage.sh <烧写文件路径>”命令,例如要烧写的led.bin位于home目录下，则烧写命令为“./mkimage.sh /home/led.bin”。

-  执行上一步后会列出linux下可烧写的磁盘，选择你插入的SD卡即可。这一步非常危险！！！一定要确定选择的是你插入的SD卡！！，如果选错很可能破坏你电脑磁盘内容，造成数据损坏！！！。确定磁盘后SD卡以“sd”开头，选择“sd”后面的字符即可。例如要烧写的sd卡是“sdb”则输入“b”即可。
