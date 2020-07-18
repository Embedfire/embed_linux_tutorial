.. vim: syntax=rst

debian系统启动过程（加载uboot、设备树、内核等）
----------


上一章我们讲解了什么是uboot、linux内核的组成结构、设备树dts以及文件系统等基本概念及其作用，
本章我们讲带领大家进行一次系统启动之旅，依次从u-boot的启动、Linux内核启动以及设备树的加载来详细分析
系统的启动流程，建议大家在阅读时对照相关源码及参考相关资料逐步分析，加深自己对u-boot启动流程的理解，为后面u-boot移植
打下坚实的基础。u-boot包含的知识面非常广，必备的知包含但不限于ARM汇编、ARMv7体系结构相关、脚本等，当然你也可以
先大概知道u-boot的工作流程，如它是如何加载环境变量的、如何加载设备树、如何加载内核并且如何将参数传递给内核等等。


uboot启动流程分析
~~~~~

PC机、Linux、Andorid启动过程对比分析
^^^^^^^

（1）PC机启动过程：
BIOS和位于硬盘MBR中的LILO和GRUB等共同构成了PC的引导加载程序，通常BIOS完成检测系统硬件（如初始化DDR内存、
硬盘）以及分配资源等工作，随后把位于硬盘MBP里的BootLoader加载到RAM中，从此BIOS便会将控制权交由这段
BootLoader（LILO和GRUB等）接管，BootLoader主要负责将内核的OS镜像从硬盘加载到RAM当中运行，之后便跳转到
内核的入口地址去运行，OS在启动之后BIOS就没有什么作用了。

（2）嵌入式Linux启动过程：
在嵌入式Linux当中，uboot以及OS都部署在flash或eMMC当中，一般在嵌入式系统中很少有和PC机类似的BIOS程序，所以系统
的启动加载任务完全由BootLoader来负责，该BootLoader所做的工作和PC机的类似，都要初始化硬件（如DDR与flash或eMMC）、
建立内存空间映射图等，接着将OS从flash或eMMC中读取到DDR中，最后启动OS，OS在启动之后u-boot也有什么作用了。

（2）Andorid系统启动过程：
由于安卓系统是基于linux系统发展而来的，所以他们的启动过程基本一致，只是在内核启动以后加载根文件系统之后有差别。
Andorid系统启动可以概括为两个阶段，第一阶段是uboot到OS，第二阶段是OS启动之后到根文件系统加载再到命令的
执行，andorid系统的启动与嵌入式Linux的主要差别为第二阶段的不同。

归纳：
嵌入式Linux系统和PC机的启动过程几乎一样，只不过是uboot替代了BIOS，flash或eMMC替代了硬盘而已，嵌入式Linux系统与andorid
更加类似，只是andorid在加载根文件系统之后有差异，三者都是类似的。


u-boot源代码情景分析
^^^^^^^

u-boot启动第一阶段源代码分析
'''''''

u-boot加载启动内核过程可以大致分为两个阶段上，详情请看上一章节，接下来我们将详细分析u-boot源代码（版本号为2019.04）。


对于imx6ull而言，其第一阶段对应的文件时arch/arm/cpu/armv7/start.S和arch/arm/cpu/armv7/lowlevel_init.S

u-boot启动第一阶段流程图如下所示：

.. image:: media/uboot_pro000.png
   :align: center
   :alt: 未找到图片00|

为了方便分析u-boot的启动流程，需要下载好u-boot源码并将其编译一遍，编译通过后我们可以看到源码根目录下
会出现一个u-boot.lds文件，如下图所示：

.. image:: media/uboot_pro001.png
   :align: center
   :alt: 未找到图片01|

如何分析u-boot.lds链接脚本？每一个链接过程都会由连接脚本（一般以lds作为文件的后缀名）控制，经过编译后的u-boot源码
会输出各个层次的链接脚本，其中总的链接脚本在u-boot源码根目录下，通过分析总的链接脚本我们可以把握u-boot的来龙去脉，
带“@”后面为注释，总的链接脚本如下所示。

.. code-block:: s
   :linenos:
   :emphasize-lines: 3,11,12

   OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")  @指定输出可执行文件是elf格式，32位ARM指令，小端
   OUTPUT_ARCH(arm)       @设置输出可执行文件的体系架构为arm
   ENTRY(_start)          @将_start设为入口地址
   SECTIONS
   {
   . = 0x00000000;        @指定可执行文件的全局入口点，通常这个地址都放在ROM(flash)0x0位置。必须使编译器知道这个地址，通常都是修改此处来完成
   . = ALIGN(4);          @代码以4字节对齐
   .text :                @指定代码段
   {
   *(.__image_copy_start)     @u-boot把自己拷贝到RAM中，这里指定拷贝的起始处
   *(.vectors)                @arch/arm/lib/vectors.S，异常向量表
   arch/arm/cpu/armv7/start.o (.text*)    @代码的第一个部分，arch/arm/cpu/armv7/start.S
   *(.text*)                              @其它代码段存放于此处
   }
   . = ALIGN(4);         @上面的代码结束后，可能会导致没有4字节对齐，这里再一次做好4字节对齐，方便后面的只读数据段
   .rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }  @指定存放只读数据段
   . = ALIGN(4);         @和上面一样，需要4字节对齐，方便后面的数据段
   .data : {             @指定读/写数据段
   *(.data*)
   }
   . = ALIGN(4);         @都一样，以后就不再赘述了
   . = .;
   . = ALIGN(4);
   .u_boot_list : {
   KEEP(*(SORT(.u_boot_list*))); @在读/写数据段后，存放一些u-boot自有的函数，如u-boot command等
   }
   . = ALIGN(4);
   .image_copy_end :
   {
   *(.__image_copy_end)          @这里指定拷贝的末尾处，拷贝的包括代码段、只读数据、读写数据段和u_boot_list等
   }
   .rel_dyn_start :                       
   {
   *(.__rel_dyn_start)           @动态链接符段开始处
   }
   .rel.dyn : {
   *(.rel*)                      @存放动态链接符的地方
   }
   .rel_dyn_end :
   {
   *(.__rel_dyn_end)             @动态链接符段末尾处
   }
   .end :
   {
   *(.__end)
   }
   _image_binary_end = .;        @二进制文件结束
   . = ALIGN(4096);
   .mmutable : {
   *(.mmutable)                  @内存管理单元表
   }
   .bss_start __rel_dyn_start (OVERLAY) : {@BSS段起始
   KEEP(*(.__bss_start));
   __bss_base = .;
   }
   .bss __bss_base (OVERLAY) : {
   *(.bss*)
      . = ALIGN(4);
      __bss_limit = .;           @把__bss_limit赋值为当前位置
   }
   .bss_end __bss_limit (OVERLAY) : {     
   KEEP(*(.__bss_end));
   }                             @BSS段末尾
   .dynsym _image_binary_end : { *(.dynsym) }
   .dynbss : { *(.dynbss) }
   .dynstr : { *(.dynstr*) }
   .dynamic : { *(.dynamic*) }
   .plt : { *(.plt*) }
   .interp : { *(.interp*) }
   .gnu.hash : { *(.gnu.hash) }
   .gnu : { *(.gnu*) }
   .ARM.exidx : { *(.ARM.exidx*) }
   .gnu.linkonce.armexidx : { *(.gnu.linkonce.armexidx.*) }
   }


到这里，我们已经带领大家详细分析了总的链接脚本u-boot.lds，大家注意.S文件中的：ENTRY(_start)，
全局搜索_start即可找到它定义在arch/arm/lib/vectors.S文件中，这里便是代码入口处，见名知意，这里便指明了异常向量，
接下来我们顺藤摸瓜来分析一下arch/arm/lib/vectors.S的执行过程。

.. code-block:: s
   :linenos:
   :emphasize-lines: 16

   *************************************************************************
   *
   * Exception vectors as described in ARM reference manuals
   *
   * Uses indirect branch to allow reaching handlers anywhere in memory.
   *
   *************************************************************************
   */

   _start:

   #ifdef CONFIG_SYS_DV_NOR_BOOT_CFG
      .word	CONFIG_SYS_DV_NOR_BOOT_CFG
   #endif

      b	resets                  /* 跳转到resets处，b为的无条件跳转，bl还把PC（r15）赋值给链接寄存器（r14） */
      ldr	pc, _undefined_instruction    /* 未定义指令异常向量 */ 
      ldr	pc, _software_interrupt       /* 预取指令异常向量 */ 
      ldr	pc, _prefetch_abort           /* 数据操作异常向量 */ 
      ldr	pc, _data_abort               /* 预取指令异常向量 */ 
      ldr	pc, _not_used                 /* 没有使用 */
      ldr	pc, _irq                      /* irq中断向量 */
      ldr	pc, _fiq                      /* fiq中断向量 */

   ......                                 /* 省略部分代码 */
   /* 中断向量表入口地址 */
   _undefined_instruction:	.word undefined_instruction  /* 当前地址（_undefined_instruction）存放undefined_instruction
   _software_interrupt:	.word software_interrupt
   _prefetch_abort:	.word prefetch_abort
   _data_abort:		.word data_abort
   _not_used:		.word not_used
   _irq:			.word irq
   _fiq:			.word fiq

      .balignl 16,0xdeadbeef

代码中断都定义了各种异常向量，当cpu产生异常时，便会将对应的异常入口地址加载到pc中，进而处理相应的异常处理程序。
各个异常向量具体描述如下表格所示：

.. csv-table:: Frozen Delights!
    :header: "地址", "异常", "进入模式", "描述"
    :widths: 15, 10, 10, 30

    "0x00000000", 复位, "管理模式", "位电平有效时，产生复位异常，程序跳转到复位处理程序处执行"
    "0x00000004", 未定义指令, "未定义模式", "遇到不能处理的指令时，产生未定义指令异"
    "0x00000008", 软件中断, "管理模式", "执行SWI指令产生，用于用户模式下的程序调用特权操作指令"
    "0x0000000c", 预存指令, "中止模式", "处理器预取指令的地址不存在，或该地址不允许当前指令访问，产生指令预取中止异常"
    "0x00000010", 数据操作, "中止义模式", "处理器数据访问指令的地址不存在，或该地址不允许当前指令访问时，产生数据中止异常"
    "0x00000014", 未使用, "未使用", "未使用"
    "0x00000018", IRQ, "IRQ", "外部中断请求有效，且CPSR中的I位为0时，产生IRQ异常"
    "0x0000001c", FIQ, "FIQ", "快速中断请求引脚有效，且CPSR中的F位为0时，产生FIQ异常"


其中复位异常向量指令“b	resets”决定了u-boot启动或者复位后将自动跳转到resets标志处执行，下面我们接着分析一下resets到底
做了哪些工作，全局搜索我们发现resets其实就定义在arch/arm/cpu/armv7/start.S文件中，如下所示：

.. code-block:: s
   :linenos:
   :emphasize-lines: 16

   /*************************************************************************
   *
   * Startup Code (reset vector)
   *
   * Do important init only if we don't start from memory!
   * Setup memory and board specific bits prior to relocation.
   * Relocate armboot to ram. Setup stack.
   *
   *************************************************************************/

      .globl	reset
      .globl	save_boot_params_ret

   reset:
      /* Allow the board to save important registers */
      b	save_boot_params
   save_boot_params_ret:


阅读上面代码，.globl意思很简单，相当于c语言当中的extern，声明reset，且告诉连接器reset为全局标量，
外部是可以访问的（在arch/arm/lib/vectors.S代码中“b	reset”有用到此变量）。分析上面代码可知，
reset中只有一条跳转指令“b save_boot_params”，搜索标号“save_boot_params”发现，它也只有一个跳转指令如下：

.. code-block:: s
   :linenos:
   :emphasize-lines: 11

   /*************************************************************************
   *
   * void save_boot_params(u32 r0, u32 r1, u32 r2, u32 r3)
   *	__attribute__((weak));
   *
   * Stack pointer is not yet initialized at this moment
   * Don't save anything to stack even if compiled with -O0
   *
   *************************************************************************/
   ENTRY(save_boot_params)
      b	save_boot_params_ret		@ 跳转到save_boot_params_ret标号处
   ENDPROC(save_boot_params)   


save_boot_params_ret标号代码如下：

.. code-block:: s
   :linenos:

   save_boot_params_ret:
	/*
	 * disable interrupts (FIQ and IRQ), also set the cpu to SVC32 mode,
	 * except if in HYP mode already
	 */
	mrs	r0, cpsr          
	and	r1, r0, #0x1f		@ mask mode bits
	teq	r1, #0x1a		   @ test for HYP mode
	bicne	r0, r0, #0x1f		@ clear all mode bits
	orrne	r0, r0, #0x13		@ set SVC mode
	orr	r0, r0, #0xc0		@ disable FIQ and IRQ
	msr	cpsr,r0

以上代码主要工作是将cpu的工作模式设置为SVC32模式（即管理模式），同时将中断禁止位与快速中断禁止位都设置为1，
以此屏蔽IRQ和FIQ的中断，说白了就是设置cpsr(Current Program Status Register)，cpsr(Saved Program Status Register)是当前程序状态寄存器，spsr是保存的程序状态寄存器。
打开《arm_architecture_reference_manual ARMv7-A and ARMv7-R edition》ARMv7架构参考手册，具体看下cpsr的位域结构，如下图所示：

.. image:: media/uboot_pro002.png
   :align: center
   :alt: 未找到图片02|

上图中红色方框标注的是“save_boot_params_ret”函数要设置的位域，其中模式位域M[4:0]决定了当前cpu的工作模式，
而位域F[6]为FIQ中断屏蔽位，位域I[7]为IRQ中断屏蔽位，位域T[5]为Thumb执行状态位（此位没有设置，可忽略），
模式位域M[4:0]详情如下表格所示：

.. image:: media/uboot_pro003.png
   :align: center
   :alt: 未找到图片03|

图中红色方框为“save_boot_params_ret”函数做过手脚的地方，我们根据Encoding来设置模式位域M[4:0]就可以设置
cpu的工作模式。
详细了解了这段位域的意思后，我们再一行一行详细的分析“save_boot_params_ret”函数，看看它到底做了什么。

1. mrs	r0, cpsr：加载cpsr寄存器的值到r0寄存器中；
2. and	r1, r0, #0x1f：屏蔽寄存器的非模式位域，留下模式位域的值；
3. teq	r1, #0x1a：测试看看当前cpu是否处于hyp模式，对照上面表格Hyp的Encoding值为11010，转成十六进制正好是0x1a；
4. bicne	r0, r0, #0x1f：清除所有的模式位M[4:0]；
5. orrne	r0, r0, #0x13：设置为Supervisor（SVC）模式，对照上面表格Supervisor的Encoding值为10011，转成十六进制正好是0x13；
6. orr	r0, r0, #0xc0： 屏蔽FIQ和IRQ中断；
7. msr	cpsr,r0： 将修改后r0寄存器的值重新装载到cpsr中。


接着继续分析后面的代码，从此我们为了方便分析源码，将源码的注释写到对应的指令行中。

.. code-block:: s
   :linenos:

   /*
   * Setup vector:
   * (OMAP4 spl TEXT_BASE is not 32 byte aligned.
   * Continue to use ROM code vector only in OMAP4 spl)
   */
   #if !(defined(CONFIG_OMAP44XX) && defined(CONFIG_SPL_BUILD))   @条件编译，如果没有定义CONFIG_OMAP44XX和CONFIG_SPL_BUILD则编译下面的代码段
      /* Set V=0 in CP15 SCTLR register - for VBAR to point to vector */
      mrc	p15, 0, r0, c1, c0, 0	@ Read CP15 SCTLR Register
      bic	r0, #CR_V		@ V = 0
      mcr	p15, 0, r0, c1, c0, 0	@ Write CP15 SCTLR Register

      /* Set vector address in CP15 VBAR register */
      ldr	r0, =_start
      mcr	p15, 0, r0, c12, c0, 0	@Set VBAR
   #endif

根据源码英文注释，我们大概知道这段代码是要设置SCTLR（系统控制寄存器），参考ARMv7架构参考手册，找到SCTLR寄存器，
具体内容如下图所示：

.. image:: media/uboot_pro004.png
   :align: center
   :alt: 未找到图片04|

SCTLR寄存器用于控制标准内存和系统设备，并且为在硬件内核中实现的功能提供状态信息，其中位域V[13]的作用是选择
异常向量表的基地址，根据ARMv7架构参考手册描述可知，当往V[13]填如0时，异常向量表的基地址=0x00000000，并且
该地址可以被re-mapped（重映射）；当往V[13]填如1时，异常向量表的基地址=0xffff0000，此时该地址不能被重映射。
源码中大量用到了mrc和mcr指令，mrc为 协处理器寄存器到ARM 处理器寄存器的数据传送指令，mcr为ARM 处理器寄存器到协处理器寄存器的数据传送指令。


1. 第8行，读取SCTLR寄存器中的值到r0中。

2. 第9行，清除SCTLR寄存器中的第CR_V位（CR_V在arch/arm/include/asm/system.h中定义为(1 << 13)），即设置异常向量表的及地址为0x00000000，且支持重映射。

3. 第10行，将修改后的r0值再写到SCTLR寄存器中。

4. 第13行，将_start的值加载到r0寄存器当中。

在u-boot源码目录下全局搜索_start（注意：要编译u-boot），在System.map文件中，可以看到_start的值为0x87800000，该地址为我们前面分析u-boot.lds中向量表vectors的起始地址，
如下图所示：

.. image:: media/uboot_pro005.png
   :align: center
   :alt: 未找到图片05|

与此同时我们还可以看到其他异常向量的链接地址，如未定义指令异常向量被链接在0x87800020中，System.map用于存放内核符号表信息。
符号表是所有符号和其对应地址的一个列表，随着每次内核的编译，就会产生一个新的对应的System.map文件，当内核运行出错时，
通过System.map中的符号表解析，就可以查到一个地址值对应的变量名。

5. 第14行，设置c12的值为0x87800000（_start），即设置非安全模式异常基址寄存器为0x87800000，结合前面清除SCTLR中的V[13]，即实现的向量表的重定位。ARM默认的异常向量表入口在0x0地址，uboot的运行介质（norflash nandflash sram等）映射地址可能不在0x0起始的地址，所以需要修改异常向量表入口。

总结：清除SCTLR中位域V[13]，然后设置VBAR指向向量表以实现向量表定位到0x87800000地址处。在分析后面代码之前，
我们先总结一下这段_start中汇编做的工作：关闭中断、初始化异常向量表、设置SVC32模式、配置cp15.

接着分析源码，如下所示：

.. code-block:: s
   :linenos:

   	/* the mask ROM code should have PLL and others stable */
   #ifndef CONFIG_SKIP_LOWLEVEL_INIT
   #ifdef CONFIG_CPU_V7A
      bl	cpu_init_cp15
   #endif
   #ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
      bl	cpu_init_crit
   #endif
   #endif

1. 第2行，如果未定义CONFIG_SKIP_LOWLEVEL_INIT则编译后面的代码块，经查找CONFIG_SKIP_LOWLEVEL_INIT未被定义，后面的代码块有效。

2. 第3行，CONFIG_CPU_V7A已被定义。

3. 第4行，跳转至cpu_init_cp15标号处，执行完cpu_init_cp15函数后返回。

4. 第6行，CONFIG_SKIP_LOWLEVEL_INIT_ONLY未被定义。

5. 第7行，跳转至cpu_init_crit标号处，执行完cpu_init_crit函数后返回。

搜索cpu_init_cp15，其函数实现如下所示：

.. code-block:: s
   :linenos:

      ENTRY(cpu_init_cp15)
      /*
      * Invalidate L1 I/D
      */
      mov	r0, #0			@ set up for MCR
      mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
      mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
      mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array
      mcr     p15, 0, r0, c7, c10, 4	@ DSB
      mcr     p15, 0, r0, c7, c5, 4	@ ISB

上面代码看起来有点复杂，但是别灰心，沉着冷静，最好是看完本章然后自己去动闹分析一边，
自然就会对u-boot有更加深入的了解，这些过程对于移植u-boot来说时非常重要的。废话不多说，咱们接着分析
上面的代码片段。

1. 第5行，这行比较简单，就是将r0寄存器的内容清零。

2. 第6行，首先，mcr做的事情其实很简单，就是“ARM处理器的寄存器中的数据传送到协处理器寄存器”，此处是，将将ARM的寄存器r0中的数据，此时r0=0,所以就是把0这个数据，传送到协处理器CP15中。而对应就是写入到C8，即将0写入到寄存器8（Register 8）中去，它是一个只写的寄存器，配合指令“mcr	p15, 0, r0, c8, c7, 0”最后两个参数，其作用是使整个数据和指令TLB无效，禁止虚拟地址到物理地址的转换，为何要关闭呢？因为刚开始我们并没有建立页表，且都是直接操作物理寄存器的，所以不能打开，否则会发生意想不到的错误。

3. 第7行，使无效整个指令缓冲。

4. 第8行，清空整个跳转目标缓冲，关闭分支预测功能。

5. 第9行，清空写缓冲区，以便数据同步。

6. 第10行，清空预取缓冲区，以便指令同步，清空流水线中已经取到的指令，进行重新取指令。

代码中基本都是清空各种缓冲，如果使用陈旧条目启用缓存，则代码可能会崩溃，导致系统无法启动。


接着往下看，下面代码都是顺序执行的，所以我们一路分析下去：

.. code-block:: s
   :linenos:

      /*
      * disable MMU stuff and caches
      */
      mrc	p15, 0, r0, c1, c0, 0
      bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
      bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
      orr	r0, r0, #0x00000002	@ set bit 1 (--A-) Align
      orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
   #ifdef CONFIG_SYS_ICACHE_OFF
      bic	r0, r0, #0x00001000	@ clear bit 12 (I) I-cache
   #else
      orr	r0, r0, #0x00001000	@ set bit 12 (I) I-cache
   #endif
      mcr	p15, 0, r0, c1, c0, 0

   #ifdef CONFIG_ARM_ERRATA_716044
      mrc	p15, 0, r0, c1, c0, 0	@ read system control register
      orr	r0, r0, #1 << 11	@ set bit #11
      mcr	p15, 0, r0, c1, c0, 0	@ write system control register
   #endif

1. 第4行，将cp15的寄存器c1的值读到r0中，c1是一个控制寄存器，它包括使能或禁止mmu以及与其他存储系统相关的功能，配置存储系统以及ARM处理器中的相关部分的工作。

2. 第5行，清除位域V[13]，即选择低端异常中断向量表，向量表基地址为0x00000000，且支持向量表重映射。

3. 第6行，清除位域M[0]、A[1]、C[2],即分别禁止内存管理单元mmu、地址对齐检查、数据缓冲。

4. 第7行，使能地址对齐检查。

5. 第8行，打开ARM系统的跳转预测（分支预测）功能，不打断流水线，提高指令执行效率。

6. 第9~13行，如果定义了CONFIG_SYS_ICACHE_OFF则关闭I-cache，否则打开I-cache，此处没有定义CONFIG_SYS_ICACHE_OFF，故打开I-cache。

7. 第14行，将修改后的r0重新写入SCTLR寄存器中。

8. 第16~20行，由于我们没有定义CONFIG_ARM_ERRATA_716044，故忽略这段代码。


到这里我们再总结一下上面这段代码的功能含义，首先，我们为何要关闭mmu？mmu负责从虚拟地址到物理地址之间的转换，但是我们现在的汇编都是直接操作物理寄存器，
此时如果打开了mmu，而我们并没有有效的TLB，这样cpu可以说是胡乱运行的，所以我们需要关闭mmu，不需要它转换地址，直接操作寄存器方便快捷。
然后，再发出灵魂拷问，为何要关闭cache？因为catch和MMU是通过cp15管理的，刚上电的时候，CPU并不能管理他们。所以上电的时候mmu必须关闭，指令cache可关闭，可不关闭，但数据cache一定要关闭，
否则可能导致刚开始的代码里面，去取数据的时候，从catch里面取，而这时候RAM中数据还没有cache过来，导致数据预取异常。

下面这些代码段大都是和cpu的cp15协处理器相关，并根据条件编译进行相关的设置，我们就不一一分析了，感兴趣的同学可以参考着《ARM ArchitectureReference Manual ARMv7-A and ARMv7-R edition》、《Cortex-A7 Technical ReferenceManua》及
《ARM Generic Interrupt Controller(ARM GIC控制器)V3.0与V4.0》等相关手册进行详细分析，我们主要的是抓住重点分析。

.. code-block:: s
   :linenos:
   :emphasize-lines: 30,32,33,35,68

   #if (defined(CONFIG_ARM_ERRATA_742230) || defined(CONFIG_ARM_ERRATA_794072))
      mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
      orr	r0, r0, #1 << 4		@ set bit #4
      mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
   #endif

   #ifdef CONFIG_ARM_ERRATA_743622
      mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
      orr	r0, r0, #1 << 6		@ set bit #6
      mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
   #endif

   #ifdef CONFIG_ARM_ERRATA_751472
      mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
      orr	r0, r0, #1 << 11	@ set bit #11
      mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
   #endif
   #ifdef CONFIG_ARM_ERRATA_761320
      mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
      orr	r0, r0, #1 << 21	@ set bit #21
      mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
   #endif
   #ifdef CONFIG_ARM_ERRATA_845369
      mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
      orr	r0, r0, #1 << 22	@ set bit #22
      mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
   #endif

      mov	r5, lr			@ 用于保存返回地址
      mrc	p15, 0, r1, c0, c0, 0	@ r1 has Read Main ID Register (MIDR)
      mov	r3, r1, lsr #20		@ get variant field
      and	r3, r3, #0xf		@ r3 has CPU variant
      and	r4, r1, #0xf		@ r4 has CPU revision
      mov	r2, r3, lsl #4		@ shift variant field for combined value
      orr	r2, r4, r2		@ r2 has combined CPU variant + revision

   #ifdef CONFIG_ARM_ERRATA_798870  @未定义，忽略此段
      cmp	r2, #0x30		@ Applies to lower than R3p0
      bge	skip_errata_798870      @ skip if not affected rev
      cmp	r2, #0x20		@ Applies to including and above R2p0
      blt	skip_errata_798870      @ skip if not affected rev

      mrc	p15, 1, r0, c15, c0, 0  @ read l2 aux ctrl reg
      orr	r0, r0, #1 << 7         @ Enable hazard-detect timeout
      push	{r1-r5}			@ Save the cpu info registers
      bl	v7_arch_cp15_set_l2aux_ctrl
      isb				@ Recommended ISB after l2actlr update
      pop	{r1-r5}			@ Restore the cpu info - fall through
   skip_errata_798870:
   #endif

   #ifdef CONFIG_ARM_ERRATA_801819  @未定义，忽略此段
      cmp	r2, #0x24		@ Applies to lt including R2p4
      bgt	skip_errata_801819      @ skip if not affected rev
      cmp	r2, #0x20		@ Applies to including and above R2p0
      blt	skip_errata_801819      @ skip if not affected rev
      mrc	p15, 0, r0, c0, c0, 6	@ pick up REVIDR reg
      and	r0, r0, #1 << 3		@ check REVIDR[3]
      cmp	r0, #1 << 3
      beq	skip_errata_801819	@ skip erratum if REVIDR[3] is set

      mrc	p15, 0, r0, c1, c0, 1	@ 读取辅助控制寄存器
      orr	r0, r0, #3 << 27	@ Disables streaming. All write-allocate
                  @ lines allocate in the L1 or L2 cache.
      orr	r0, r0, #3 << 25	@ Disables streaming. All write-allocate
                  @ lines allocate in the L1 cache.
      push	{r1-r5}			@ 保存参数信息，用于传递参数给v7_arch_cp15_set_acr函数
      bl	v7_arch_cp15_set_acr      @跳转到v7_arch_cp15_set_acr函数中，其函数声明为：void __weak v7_arch_cp15_set_acr(u32 acr, u32 cpu_midr, u32 cpu_rev_comb, u32 cpu_variant, u32 cpu_rev)
      pop	{r1-r5}			@ Restore the cpu info - fall through
   skip_errata_801819:
   #endif

   #ifdef CONFIG_ARM_ERRATA_454179  CONFIG_ARM_ERRATA_798870 @未定义，忽略此段
      cmp	r2, #0x21		@ Only on < r2p1
      bge	skip_errata_454179

      mrc	p15, 0, r0, c1, c0, 1	@ Read ACR
      orr	r0, r0, #(0x3 << 6)	@ Set DBSM(BIT7) and IBE(BIT6) bits
      push	{r1-r5}			@ Save the cpu info registers
      bl	v7_arch_cp15_set_acr
      pop	{r1-r5}			@ Restore the cpu info - fall through

   skip_errata_454179:
   #endif

   #ifdef CONFIG_ARM_ERRATA_430973  @未定义，忽略此段
      cmp	r2, #0x21		@ Only on < r2p1
      bge	skip_errata_430973

      mrc	p15, 0, r0, c1, c0, 1	@ Read ACR
      orr	r0, r0, #(0x1 << 6)	@ Set IBE bit
      push	{r1-r5}			@ Save the cpu info registers
      bl	v7_arch_cp15_set_acr
      pop	{r1-r5}			@ Restore the cpu info - fall through

   skip_errata_430973:
   #endif

   #ifdef CONFIG_ARM_ERRATA_621766  @没有定义，忽略此段
      cmp	r2, #0x21		@ Only on < r2p1
      bge	skip_errata_621766

      mrc	p15, 0, r0, c1, c0, 1	@ Read ACR
      orr	r0, r0, #(0x1 << 5)	@ Set L1NEON bit
      push	{r1-r5}			@ Save the cpu info registers
      bl	v7_arch_cp15_set_acr
      pop	{r1-r5}			@ Restore the cpu info - fall through

   skip_errata_621766:
   #endif

      mov	pc, r5			@ 返回
   ENDPROC(cpu_init_cp15)

上面的代码有个很有意思的地方，来和大家分享一下，有没有发现代码中重复出现以下代码段：

.. code-block:: s
   :linenos:
   :emphasize-lines: 1,3

   push	{r1-r5}			@ Save the cpu info registers
   bl	v7_arch_cp15_set_acr
   pop	{r1-r5}			@ Restore the cpu info - fall through

笔者分析，这是典型的函数调用过程，全局搜索v7_arch_cp15_set_acr，在arch/arm/cpu/armv7/cp15.c有该函数的实现，如下所示：

.. code-block:: c
   :linenos:
   :emphasize-lines: 4

   void __weak v7_arch_cp15_set_acr(u32 acr, u32 cpu_midr, u32 cpu_rev_comb,
				 u32 cpu_variant, u32 cpu_rev)
   {
      asm volatile ("mcr p15, 0, %0, c1, c0, 1\n\t" : : "r"(acr));
   }

这是一段内嵌汇编函数，该函数主要作用是设置ACTLR（辅助控制寄存器），关于ACTLR的具体描述，大家可以参考e
《Cortex-A7 Technical ReferenceManua》，我们分析一下它是如何进行函数调用以及参数传递的。首先函数v7_arch_cp15_set_acr调用之前都进行了push入栈操作，它是将{r1-r5}这五个寄存器都压入堆栈中，
不同于x86的参数传递规则，ARM程序调用规则ATPCS（ARM-Thumb Procedure Call Standard）建议函数的形参不超过4个，如果形参个数少于或等于4，则形参由R0,R1,R2,R3四个寄存器进行传递；若形参个数大于4，大于4的部分必须通过堆栈进行传递。
显然函数v7_arch_cp15_set_acr共有5个参数acr、cpu_midr、cpu_rev_comb、cpu_variant、cpu_rev，所以大于4的部分必须通过堆栈进行传递，而这里是将{r1-r5}全都入栈了，其中{r0-r4}5个寄存器的值分别作为v7_arch_cp15_set_acr函数的5个参数
来传递。第4行，v7_arch_cp15_set_acr函数中有一段内嵌汇编代码，其中“%0”就是变量acr的值也就是r0寄存器中的值，也就是ACTLR寄存器，因为前面已经将ACTLR读入到r0中，并且改变了r0的值。r1存储的是MIDR（Main ID Register），
MIDR提供处理器的标识信息，包括设备的实现代码和设备ID号，MIDR和其他寄存器（{r2-r4}）的值都没有用到，所以我们就不追究了，大概知道其调用规则即可。函数调用完后需要将调用前入栈的数据给pop（出栈）掉。

到此cpu_init_cp15函数基本上分析完了，接下来继续分析cpu_init_crit函数，代码如下：

.. code-block:: c
   :linenos:
   :emphasize-lines: 1,17

   #ifndef CONFIG_SKIP_LOWLEVEL_INIT
   /*************************************************************************
   *
   * CPU_init_critical registers
   *
   * setup important registers
   * setup memory timing
   *
   *************************************************************************/
   ENTRY(cpu_init_crit)
      /*
      * Jump to board specific initialization...
      * The Mask ROM will have already initialized
      * basic memory. Go here to bump up clock rate and handle
      * wake up conditions.
      */
      b	lowlevel_init		@ go setup pll,mux,memory
   ENDPROC(cpu_init_crit)
   #endif

1. 第1行，如果没有定义CONFIG_SKIP_LOWLEVEL_INIT，则编译cpu_init_crit相关代码段，源码中搜索CONFIG_SKIP_LOWLEVEL_INIT，发现其确实未被定义。

2. 第17行，跳转至lowlevel_init函数执行。

由于lowlevel_init完成了内存的初始化工作，而内存的初始化依赖于开发板，所以lowlevel_init于当前imx6u相关，
lowlevel_init函数是与特定开发板相关的初始化函数，在这个函数里会做一些pll初始化，
如果不是从内存启动，则会做内存初始化，方便后续拷贝到内存中运行。
全局搜索lowlevel_init发现其在arch/arm/cpu/armv7/lowlevel_init.S文件中有定义：

.. code-block:: s
   :linenos:
   :caption: arch/arm/cpu/armv7/lowlevel_init.S
   :emphasize-lines: 8

   WEAK(lowlevel_init)
      /*
      * Setup a temporary stack. Global data is not available yet.
      */
   #if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
      ldr	sp, =CONFIG_SPL_STACK
   #else
      ldr	sp, =CONFIG_SYS_INIT_SP_ADDR
   #endif
      bic	sp, sp, #7 /* 8-byte alignment for ABI compliance */
   #ifdef CONFIG_SPL_DM
      mov	r9, #0
   #else
      /*
      * Set up global data for boards that still need it. This will be
      * removed soon.
      */
   #ifdef CONFIG_SPL_BUILD
      ldr	r9, =gdata
   #else
      sub	sp, sp, #GD_SIZE
      bic	sp, sp, #7
      mov	r9, sp
   #endif
   #endif
      /*
      * Save the old lr(passed in ip) and the current lr to stack
      */
      push	{ip, lr}

      /*
      * Call the very early init function. This should do only the
      * absolute bare minimum to get started. It should not:
      *
      * - set up DRAM
      * - use global_data
      * - clear BSS
      * - try to start a console
      *
      * For boards with SPL this should be empty since SPL can do all of
      * this init in the SPL board_init_f() function which is called
      * immediately after this.
      */
      bl	s_init
      pop	{ip, pc}
   ENDPROC(lowlevel_init)

忽略不符合条件编译的代码。

1. 第8行，设置栈指针指向CONFIG_SYS_INIT_SP_ADDR，而CONFIG_SYS_INIT_SP_ADDR具体是什么？它在include/configs/mx6ullevk.h文件中有如下定义：

.. code-block:: s
   :linenos:
   :caption: include/configs/mx6ullevk.h
   :emphasize-lines: 8

   /* Physical Memory Map */
   #define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

   #define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
   #define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
   #define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

   #define CONFIG_SYS_INIT_SP_OFFSET \
      (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
   #define CONFIG_SYS_INIT_SP_ADDR \
      (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

上述代码段可以总结出：CONFIG_SYS_INIT_SP_ADDR = IRAM_BASE_ADDR + （IRAM_SIZE - GENERATED_GBL_DATA_SIZE），
IRAM_BASE_ADDR在arch/arm/include/asm/arch-mx6/imx-regs.h定义为0x00900000，
IRAM_SIZE在arch/arm/include/asm/arch-mx6/imx-regs.h中定义为0x00020000，如下所示：

.. code-block:: s
   :linenos:
   :caption: arch/arm/include/asm/arch-mx6/imx-regs.h
   :emphasize-lines: 2,6

   #if !(defined(CONFIG_MX6SX) || \
      defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL) || \
      defined(CONFIG_MX6SLL) || defined(CONFIG_MX6SL))
   #define IRAM_SIZE                    0x00040000
   #else
   #define IRAM_SIZE                    0x00020000
   #endif

在.config文件中我们配置了CONFIG_MX6ULL=y，所以条件不成立，即定义IRAM_SIZE = 0x00020000，.config部分配置文件如下所示：

.. code-block:: .config
   :linenos:
   :caption: .config
   :emphasize-lines: 5

   CONFIG_SYS_TEXT_BASE=0x87800000
   CONFIG_SYS_MALLOC_F_LEN=0x400
   # CONFIG_SECURE_BOOT is not set
   CONFIG_MX6=y
   CONFIG_MX6ULL=y
   CONFIG_LDO_BYPASS_CHECK=y
   # CONFIG_MODULE_FUSE is not set
   # CONFIG_TARGET_ADVANTECH_DMS_BA16 is not set

由此可见.config配置文件与我们的代码息息相关，大家可修改.config中的配置项来定制我们自己的u-boot，比如
修改CONFIG_BOOTDELAY=3，可以设置uboot启动延时；修改CONFIG_BAUDRATE=115200设置串口波特率。当然我们可以使用更人性化
的make menuconfig来配置u-boot，配置好后，就会在u-boot根目录下生成最新的.config文件，u-boot根据这些配置文件来决定该如何编译
u-boot源码，不多说了，继续往下分析。

GENERATED_GBL_DATA_SIZE在include/generated/generic-asm-offsets.h中定义为256，转换成十六进制为0x00000100。
故CONFIG_SYS_INIT_SP_ADDR = （0x00900000 + （0x00020000 - 0x00000100）） = 0x0091ff00。

2. 第10行，根据英文注释可知，它是要遵从ABI的8字节对齐。

3. 第21行，将堆栈指针减去GD_SIZE，GD_SIZE在include/generated/generic-asm-offsets.h中被定义为256，即sp = sp - 0x00000100.

4. 第22行，和上面一样遵从ABI的8字节对齐。

5. 第23行，将sp的值存储在r9寄存器当中。

6. 第29行，将ip和pc压入栈中。

7. 第44行，调用s_init函数。

8. 第45行，将ip和pc出栈，入栈出栈是函数调用的常规操作，大家习惯就好。

全局搜索s_init，发现s_init函数为一个空的函数，里面什么也没做，如下所示：

.. code-block:: c
   :linenos:
   :caption: arch/arm/cpu/armv7/ls102xa/soc.c

   void s_init(void)
   {
   }

所以到此lowlevel_init函数就大致分析完了，lowlevel_init函数返回后，我们又回到了最初arch/arm/cpu/armv7/start.S文件中
cpu_init_crit的返回处，即接下来将进入_main函数。

.. code-block:: s
   :linenos:
   :caption: arch/arm/cpu/armv7/start.S

   #ifndef CONFIG_SKIP_LOWLEVEL_INIT_ONLY
	bl	cpu_init_crit
   #endif
   #endif

      bl	_main

全局搜索_main，发现它在arch/arm/lib/crt0.S中有定义：

.. code-block:: s
   :linenos:
   :caption: arch/arm/lib/crt0.S
   :emphasize-lines: 21

   ENTRY(_main)

   /*
   * Set up initial C runtime environment and call board_init_f(0).
   */

   #if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_STACK)
      ldr	r0, =(CONFIG_SPL_STACK)
   #else
      ldr	r0, =(CONFIG_SYS_INIT_SP_ADDR)
   #endif
      bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
      mov	sp, r0
      bl	board_init_f_alloc_reserve
      mov	sp, r0
      /* set up gd here, outside any C code */
      mov	r9, r0
      bl	board_init_f_init_reserve

      mov	r0, #0
      bl	board_init_f

看到_main是否有一种莫名的熟悉感？其实我们在stm32中的startup_stm32f10x_hd.s文件中也能看到_main的身影，
其实它们都有点类似。上面代码中主要是初始化c语言的运行环境，总所周知，c的运行依赖函数的调用及传参等，所以不可或缺的要用到
堆栈。

1. 第7行，不满足条件编译，忽略。

2. 第10行，加载CONFIG_SYS_INIT_SP_ADDR到r0寄存器，CONFIG_SYS_INIT_SP_ADDR的值我们在前面已经计算过了，这里就不重复了，
CONFIG_SYS_INIT_SP_ADDR = 0x0091ff00，详情参考include/configs/mx6ullevk.h文件。

3. 第12行，遵从ABI的8字节对齐，为什么要保证堆栈8字节对齐？AAPCS规则要求堆栈保持8字节对齐。如果不对齐，调用一般的函数也是没问题的。但是当调用需要严格遵守AAPCS规则的函数时可能会出错。例如调用sprintf输出一个浮点数时，栈必须是8字节对齐的，否则结果可能会出错。

4. 第13行，将堆栈指针指向r0寄存器的值，由于r0本就是对齐的，所以sp=0x0091ff00。

5. 第14行，调用board_init_f_alloc_reserve函数，该函数有一个参数top，根据ARM函数调用规则，top=r0=0x0091ff00，该函数主要作用是保留早期malloc区域，且为GD（全局数据区）留出空间，函数返回值也是r0，r0保存着预留早期malloc区域和GD后的地址，r0 = 0x0091ff00 - (0x400（early malloc arena） + 0x100（GD_SIZE）) = 0x0091fa00，详情查阅common/init/board_init.c。

6. 第17行，根据英文注释，即设置GD为r0的值，即GD地址为0x0091fa00，r9是gd全局变量的指针，如下所示：

.. code-block:: s
   :linenos:
   :caption: arch/arm/include/asm/global_data.h

   #ifdef CONFIG_ARM64
   #define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("x18")
   #else
   #define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("r9")
   #endif

7. 第18行，调用board_init_f_init_reserve函数，该函数主要作用是将GD区域清零，返回最初malloc区域的地址，即 0x0091fb00 =  0x0091fa00 + 0x100（GD_SIZE）。

8. 第20~22行，清空r0，然后把参数r0传给board_init_f函数，并调用board_init_f。


总结：初始化c语言环境，以便调用board_init_f函数。这个环境只提供了一个堆栈和一个存储GD（全局数据）结构的地方，两者都位于一些可用的RAM中。在调用board_init_f()之前，GD应该被归零。

接着分析board_init_f：

.. code-block:: c
   :linenos:
   :caption: common/board_f.c

   void board_init_f(ulong boot_flags)
   {
      gd->flags = boot_flags;
      gd->have_console = 0;

      if (initcall_run_list(init_sequence_f))
         hang();

   #if !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
         !defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
         !defined(CONFIG_ARC)
      /* NOTREACHED - jump_to_copy() does not return */
      hang();
   #endif
   }

1. 第3行，设置dg的标志为0，boot_flags是board_init_f函数调用前r0的值（0x0）。

2. 第4行，标记dg的have_console为0，表示我们还没有初始化控制台，dg的结构体gd_t定义在include/asm-generic/global_data.h中。

3. 第5行，调用initcall_run_list（）初始化uboot的前半段。


接着我们分析一下initcall_run_list。

.. code-block:: c
   :linenos:

   DECLARE_GLOBAL_DATA_PTR;

   static inline int initcall_run_list(const init_fnc_t init_sequence[])
   {
      const init_fnc_t *init_fnc_ptr;

      for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
         unsigned long reloc_ofs = 0;
         int ret;

         if (gd->flags & GD_FLG_RELOC)
            reloc_ofs = gd->reloc_off;
   #ifdef CONFIG_EFI_APP
         reloc_ofs = (unsigned long)image_base;
   #endif
         debug("initcall: %p", (char *)*init_fnc_ptr - reloc_ofs);
         if (gd->flags & GD_FLG_RELOC)
            debug(" (relocated to %p)\n", (char *)*init_fnc_ptr);
         else
            debug("\n");
         ret = (*init_fnc_ptr)();
         if (ret) {
            printf("initcall sequence %p failed at call %p (err=%d)\n",
                  init_sequence,
                  (char *)*init_fnc_ptr - reloc_ofs, ret);
            return -1;
         }
      }
      return 0;
   }


