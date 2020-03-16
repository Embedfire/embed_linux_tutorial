核心转储调试
========

核心转储——core文件简介
----------------------

核心转储文件（core文件，也被称之为core
dump文件，可能某些书籍上称之为“内核转储文件”，都是一样的，不必纠结名称）
是操作系统在进程收到某些信号而终止运行时，将此时进程地址空间的内容以及有关进程状态的其他信息写入一个磁盘文件，
这个文件就是核心转储文件，它里面包含了进程崩溃时的所有信息，比如包含终止时进程内存的映像、寄存器状态，堆栈指针，
内存管理信息等，而这种信息往往用于调试，比如在gdb调试器使用，导致进程终止运行的信号可以在系统的signal列表中找到。

为什么要学这个东西呢，因为是程序员总会写bug的，我们的程序一旦运行，一开始可能没什么问题，但是谁能保证一个
很大的工程没有一点问题呢，那么如果我们使用了核心转储，在程序运行崩溃时我们就能找到程序出现问题的地方，对于
调试还是解bug都是非常好的，虽然程序崩溃了，但是它留下来现场的数据啊，就跟开车撞了别人一样，被监控拍到了，
就能知道当时发生了什么事情。

简单的说，在一个程序崩溃时，系统会在指定目录下生成一个core文件，core文件主要是用来调试的。

打开或关闭core文件的生成
------------------------

在默认情况下，系统是没有打开core文件生成的，比如我的服务器ubuntu18.04就是没有打开core文件生成的功能，
当程序崩溃了我也不知道它哪里出现问题，那么我们可以通过以下命令打开core文件的生成：

.. code:: bash

    # 改为无限制大小
    ulimit -c unlimited     

如果想检验你们的系统是否打开了core文件的生成，那么可以通过以下命令判断，如果终端打印的值是0，则代表系统并未开启
生成core文件的功能，而如果是其他数值：比如1024，这代表系统最大可以产生1024字节的core文件，而如果是unlimited则
代表系统不限制core文件的大小，只要有足够的磁盘空间，可以产生10G、100G、甚至是10000G的core文件，当然啦，你的程序
有足够大才行。

.. code:: bash

    # 检验是否打开core文件生成
    ulimit -c  

    # core文件的大小
    unlimited

限制系统产生core文件的大小为1024000字节：

.. code:: bash

    # 改为限制大小1024000字节
    ulimit -c 1024000     

    ps：限制其他大小字节就在命令中修改对应的数值即可

设置永久生效
------------

上面所述的方法只是在当前终端生效，当你打开了一个新的终端，它是无效的，也就是说你在其他终端运行程序时，
即使程序崩溃了，系统也不会产生core文件，那么如果我想要设置使设置让它永久生效的办法怎么办呢？

很简单，设置永久生效的办法是在\ ``profile``\ 文件中添加一句代码\ ``ulimit -c xxxx``\ 即可：

.. code:: bash

    # 命令，打开profile文件
    vi /etc/profile

    # 添加以下代码，数值可以是其他值
    ulimit -c 1024000 

    # 或者是
    ulimit -c unlimited

这样重启机器后就生效了。也可以使用\ ``source``\ 命令使之马上生效。

.. code:: bash

    source /etc/profile

不生成核心转储文件的情况
------------------------

linux系统在多种情况下不会生成核心转储文件：

-  该进程无权写入核心转储文件。在默认情况下，产生的核心转储文件名称为core或core.pid，其中pid是转储核心转储的进程的ID，并在当前进程的工作目录中创建，而如果核心转储文件创建失败或者该目录不可写，或者存在相同名称的文件且该文件是不可写的（它是目录或符号链接），那么在进程奔溃时将无法产生核心转储文件。

-  已经存在一个与可用于核心转储的名称相同的（可写的，常规的）文件，但是该文件有多个硬链接。

-  用于创建核心转储文件的文件系统已满，存放不下心产生的core文件

-  文件系统是只读形式的。

-  要在其中创建核心转储文件的目录不存在。

-  系统中核心转储文件大小资源限制设置为0，或者核心转储文件大小已经超过core文件限制的上限。

指定核心转储的文件名和目录
~~~~~~~~~~~~~~~~~~~~~~~~~~

在默认情况下，系统在进程崩溃时产生的core文件是存在与该进程的程序文件相同的目录下的，并且固定命名为core，
如此此时系统中有多个程序文件都存放在同一个目录下，而恰巧有多个进程崩溃，那么产生的core文件就会相互覆盖，
而如果我们想要分析他们，那就没法去分析了，因此我们可以通过修改配置，让产生的核心转储文件命名包含相应的信息，
而不会导致覆盖，也可以指定核心转储文件的路径，如何做到呢？

只需在\ ``/etc/sysctl.conf``\ 文件中，设置kernel.core_pattern的值即可，具体操作如下：

1. 使用vi编辑器打开\ ``/etc/sysctl.conf``\ 文件：

.. code:: bash

    vi /etc/sysctl.conf

2. 在文件末尾添加以下代码：

.. code:: bash

    kernel.core_pattern = core_%e_%p 
    kernel.core_uses_pid = 0

这代表着在当前目录下产生core文件，而如果想在其他目录下产生core文件，也是可以指定对应的目录的，如：

.. code:: bash

    # 在/var/core目录下产生core文件
    kernel.core_pattern = /var/core/core_%e_%p 
    kernel.core_uses_pid = 0

其中core_pattern的配置中%e, %p分别代表以下参数：

-  %e：所dump的文件名
-  %p：所dump的进程PID
-  %c：转储文件的大小上限
-  %g：所dump的进程的实际组ID
-  %h：主机名
-  %s：导致本次coredump的信号
-  %t：转储时刻(由1970年1月1日起计的秒数)
-  %u：所dump进程的实际用户ID

需要注意的是，如果\ ``/proc/sys/kernel/core_uses_pid``\ 文件的内容被设置为1，即使\ ``kernel.core_pattern``\ 中没
有设置%p，最后生成的core文件名仍会加上进程ID。

然后可以使用以下命令，使修改结果马上生效。

.. code:: bash

    sudo /sbin/sysctl -p

    注意，当你指定了\ ``kernel.core_pattern``\ 路径的时候，如果没有足够的权限，那么是不能生成core文件的。
    可能就需要sudo权限来运行程序了。

强制某个进程产生core dump
-------------------------

在日常写代码的时候，我们写了一些bug不一定会导致进程崩溃，而是可能会让进程卡在某个地方，比如发生看死锁，此时程序已经
是不正常运行了，而我们还不知道进程的错误在哪里，如果开发的环境又没有gdb调试，那么我们可以尝试在外部让进程崩溃，
从而产生core文件，根据linux的信号默认的处理行为，\ ``SIGQUIT，SIGABRT, SIGFPE和SIGSEGV``\ 都可以让该进程产生core文件，
那么我们可以手动发送这些信号让进程终止并且产生core文件，前提是进程没有处理这些信号。

还有一种方法，在你认为程序可能出现卡死的地方主动调用\ ``abort()``\ 函数产生core文件，这个函数首先取消阻止SIGABRT信号，
然后为调用进程引发该信号（就像调用了\ ``raise()``\ 函数一样），除此之外还有可以使用gdb调试工具来产生core文件。

使用gdb调试core文件
-------------------

学习了那么多，现在来进行一波实战操作，让大家熟悉一下流程，在产生core文件的时候就可以自己去调试了。

首先我们需要产生一个core文件对吧，那就写一个bug咯，写代码我不会，难道写bug我还不会吗，是吧，看以下代码：

.. code:: c

    #include <sys/types.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>

    int main(void)
    {
        int *a = NULL;

        printf("这是一个错误\n");

        // abort();
        *a = 0x1;

        printf("看看我是否能打印出来\n");

        sleep(1);       // 防止进程过快退出

        return 0;
    }

代码定义了一个指向NULL的指针，然后对指针操作，那肯定是内存越界了，进程崩溃后会产生一个core文件，
这不就可以调试了吗!

代码的路径是在：\ ``embed_linux_tutorial/documentation/linux_debug``\ ，先make编译代码，然后直接运行：

.. code:: bash

    ➜  core_dump git: ✗ make 
    gcc -o core_dump.o -c -g -Werror -I. -Iinclude -static core_dump.c -g -MD -MF .core_dump.o.d
    gcc -o targets core_dump.o -g -Werror -I. -Iinclude -static

    ➜  core_dump git: ✗ ls
    core_dump.c  core_dump.o  Makefile  targets

    ➜  core_dump git: ✗ ./targets 
    这是一个错误
    [1]    19176 segmentation fault (core dumped)  ./targets

    ➜  core_dump git: ✗ ls
    core_dump.c  core_dump.o  core_targets_19176  Makefile  targets

在运行时可以看到输出了一个错误，\ ``[1]    19176 segmentation fault (core dumped)  ./targets``\ ，告诉我们
产生了一个core文件，那么在当前目录下就产生了\ ``core_targets_19176``\ 文件，那么怎么来调试呢，通过以下命令即可：

.. code:: bash

    ➜  core_dump git:(dev_jie) ✗ gdb targets core_targets_19176 

    GNU gdb (Ubuntu 8.1-0ubuntu3.2) 8.1.0.20180409-git
    Copyright (C) 2018 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
    and "show warranty" for details.
    This GDB was configured as "x86_64-linux-gnu".
    Type "show configuration" for configuration details.
    For bug reporting instructions, please see:
    <http://www.gnu.org/software/gdb/bugs/>.
    Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.
    For help, type "help".
    Type "apropos word" to search for commands related to "word"...
    Reading symbols from targets...done.
    [New LWP 19176]
    Core was generated by `./targets'.
    Program terminated with signal SIGSEGV, Segmentation fault.
    #0  0x0000000000400b6d in main () at core_dump.c:13
    ---Type <return> to continue, or q <return> to quit---
    13          *a = 0x1;
    (gdb) 

命令格式如下：

.. code:: bahs

    gdb [程序文件] [core文件]

在gdb调试的底部，我们可以看到错误的位置就是第13行，对空指针操作的位置，gdb工具直接
定位出来错误在哪，还不需要我们一步步调试。

.. code:: bash

    Program terminated with signal SIGSEGV, Segmentation fault.
    #0  0x0000000000400b6d in main () at core_dump.c:13
    13          *a = 0x1;

如果我们想一步步调试，那么可以在gdb中打断点(b)，然后运行(r)，或者一步步(s)调试，具体见：

.. code:: bash

    Program terminated with signal SIGSEGV, Segmentation fault.
    #0  0x0000000000400b6d in main () at core_dump.c:13
    13          *a = 0x1;

    # 下面是gdb调试的步骤：
    # 首先在main函数中打一个断点
    (gdb) b main
    Breakpoint 1 at 0x400b55: file core_dump.c, line 8.

    # 运行到断点处
    (gdb) r
    Starting program: /home/jiejie/embed_linux_tutorial/base_code/linux_debug/core_dump/targets 

    Breakpoint 1, main () at core_dump.c:8
    8           int *a = NULL;

    # 接下来单步运行
    (gdb) s
    10          printf("这是一个错误\n");
    (gdb) s
    这是一个错误
    13          *a = 0x1;

    # 运行到第13行这里就出现错误了
    (gdb) s

    Program received signal SIGSEGV, Segmentation fault.
    0x0000000000400b6d in main () at core_dump.c:13
    13          *a = 0x1;

    # 退出gdb调试
    (gdb) quit
    A debugging session is active.

            Inferior 1 [process 19261] will be killed.

    Quit anyway? (y or n) y

    本章完，大家在本章练习中可以随意写bug然后调试，都是可以的~
