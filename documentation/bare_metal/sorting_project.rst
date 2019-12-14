.. vim: syntax=rst

整理工程文件
------

本章节内容处于开发笔记状态，还待整理至最终版的教程。

本章节内容处于开发笔记状态，还待整理至最终版的教程。

为降低难度，在之前章节我们把程序所有内容放在一个文件夹下。这样做存在两个主要问题，第一，代码存放混乱不易阅读。第二，程序可移植性差。本章将对代码进行简单的整理，让程序存放更有条理。

本章主要内容：

-  创建文件夹分类存储代码

-  修改makefile

分类存储代码
~~~~~~

本章程序基于按键检测程序。修改后的程序结构如图 53‑1所示。

|sortin002|

图 53‑1调整后的按键程序结构

结合图 53‑1说明如下：

-  base.lds，将button.lds更改为base.lds。连接脚本与具体的外设无关，并且后面的程序我们几乎不会去修改链接脚本，这里把它修改为base.lds，你也可以根据自己喜好命令，只要和makefile中一致即可。

-  device文件夹，保存外设驱动的源文件（.c文件）。

-  include文件夹，保存程序中使用的头文件。

-  makefile，主makefile 。在主makefile中会调用“device”文件夹下的子makefile.

修改makefile
~~~~~~~~~~

修改makefile主要包括两部分，第一部分，在“device”文件夹下添加并编写子makefile，第二部分，修改主makefile。

编写子makefile
^^^^^^^^^^^

子makefile用于将“device”文件夹下的驱动源文件编译为一个“.o”文件，完整内容如代码清单 53‑1所示。

代码清单 53‑1device文件夹下的子makefile

1 /第一部分/

2 all : button.o led.o （1）

3 arm-none-eabi-ld -r $^ -o device.o

4

5 /第二部分/

6 %.o : %.c

7 arm-none-eabi-gcc ${CFLAGS} -c $^

8

9 /第三部分/

10 %.o : %.S

11 arm-none-eabi-gcc ${CFLAGS} -c $^

12

13 .PHONY: clean

14 clean:

15 -rm -f \*.o \*.bak

16

17 /第四部分/

18 #定义变量,用于保存编译选项和头文件保存路径

19 header_file := -fno-builtin -I$(shell pwd)/include

20 export header_file

结合代码，各部分简单说明如下：

-  第一部分，链接生成最终目标“device.o”。如果程序中新增了某个外设驱动程序，只需要将对应的“.o”文件填入“依赖”处即可。

-  第二部分，编译“device”文件夹下的所有“.c”文件并生成对应的“.o”文件。

-  第三部分，编译“device”文件夹下的所有“.S”文件并生成对应的“.o”文件。

-  第四部分，它是定义在主makefile中的变量，用“export”声明后可以在其他makefile中调用。

修改主makefile
^^^^^^^^^^^

主makefile的改动主要有亮点，第一，在编译命令中指明头文件位置，第二，使用命令调用子makefile，生成依赖文件。完整的代码如代码清单 53‑2所示。

代码清单 53‑2主makefile文件

1 /第一部分/

2 #定义变量，用于保存编译选项和头文件保存路径

3 header_file := -fno-builtin -I$(shell pwd)/include

4 export header_file

5

6 /第二部分/

7 all : start.o main.o device/device.o

8 arm-none-eabi-ld -Tbase.lds $^ -o base.elf

9 arm-none-eabi-objcopy -O binary -S -g base.elf base.bin

10

11 /第三部分/

12 %.o : %.S

13 arm-none-eabi-gcc -g -c $^

14 %.o : %.c

15 arm-none-eabi-gcc $( header_file) -c $^

16

17 /第四部分/

18 #调用其他文件的makefile

19 device/device.o :

20 make -C device all

21

22

23 /第五部分/

24 #定义清理伪目标

25 .PHONY: clean

26 clean:

27 make -C device clean

28 -rm -f \*.o \*.elf \*.bin

结合代码，各部分简单说明如下：

-  第一部分，定义变量 “CFLAGS”。在makefile中“变量”更像C原因中的宏定义。“-fno-builtin”是一个编译选项，用于解决库函数与自己编写函数同名问题。“-I$(shell pwd)/include”用于指定头文件路径。

-  第二部分，makefile最终目标，与原文件相比，这里只是修改了文件的名字。

-  第三部分，定义生成“.o”文件的相关内容

-  第四部分，定义生成“device/device.o”的命令，“device.o”文件由子makefile生成，所以这里只需要调用子makefile即可。

-  第五部分，定义清理伪目标。在清理命令中不但要清理主makefile所在文件夹的内容还要调用子makefile的清理命令以清理子makefile所在文件夹的内容。

.. |sortin002| image:: media/sortin002.png
   :width: 4.13913in
   :height: 2.28883in
