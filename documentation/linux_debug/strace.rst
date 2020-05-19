跟踪系统调用——strace
====================

在前面的文章我们了解了gdb调试工具，今天我们来了解一下新的调试工具，它虽然不能与
gdb一样跟踪程序的运行，调试程序运行时的环境，如查看栈的内容与函数内部的变量等信
息，但它依旧是一个非常强大的工具，我们一般用来跟踪任何程序的系统调用，没错，是任何
程序，比如Linux系统中自带的命令，或者是其他不开源的可执行文件，strace都可以跟踪他
们的系统调用，下面我们就来简单了解一下strace这个工具。

strace简介
----------

strace命令是一个集诊断、调试、统计与一体的工具，我们用它来监控用户空间进程和内核
的交互，比如对应用程序的系统调用、信号传递与进程状态变更等进行跟踪与分析，以达到解
决问题或者是了解应用工作过程的目的。这个简单而又强大的工具几乎在所有的Linux操作系
统上都可用，足以见证其强大之处。

我们知道Linux的前身是Unix，Unix的设计由大量的函数调用组成，比如包括一些简单的操
作，打开一个文件，读写文件，关闭文件等，这写就是系统调用，而Linux也是如此，拥有大
量系统提供的函数接口。在Linux世界，进程不能直接访问硬件设备，当进程需要访问硬件设
备（比如读取磁盘文件，接收网络数据等等）时，必须由用户态模式切换至内核态模式，通过
系统调用访问硬件设备。所有的Linux应用程序都是通过调用操作系统提供的函数来完成它们
的任务，这也是我们常说的应用编程，我们不需要去了解操作系统到底在干嘛，我们只需要知
道通过这些系统调用就可以完成我们的操作，比如向串口写入数据，那么就将串口设备打开，
然后写入数据，最后关闭串口设备等。

那么如果程序出现问题，我们不知道问题出现在哪里，而strace工具就派上用场了，使用
strace工具，我们可以清楚地看到这些系统调用的过程、使用的参数、返回值与执行消耗的
时间等，还可以了解它们与操作系统之间的底层交互。

    ps：strace底层使用内核的ptrace特性来实现其功能，有兴趣的同学可以去了解一下ptrace。

strace命令
----------

strace是系统自带的命令，在绝大部分的linux系统中可以用它来调试应用程序，只需我们运
行一下strace命令就可以跟踪某个应用程序，在应用程序运行结束后它也会自动结束跟踪，
在命令执行的过程中，strace会记录和解析命令进程的所有系统调用以及这个进程所接收到
的所有的信号值，通过这些跟踪到的信息，你可以知道程序执行了那些系统调用，以何种顺序
执行，收到了什么信号等。

在linux下运行 ``strace -h`` 命令就可以看到strace命令用法的介绍，还有很多参数，由于太
多就不一一列举出来。

.. code:: bash

    ➜  ~ strace -h

    usage: strace [-CdffhiqrtttTvVwxxy] [-I n] [-e expr]...
                  [-a column] [-o file] [-s strsize] [-P path]...
                  -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]
       or: strace -c[dfw] [-I n] [-e expr]... [-O overhead] [-S sortby]
                  -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]

**strace命令输出的格式参数：**

    ps：这些参数挑一些常用的记就好了，因为如果不记得运行 ``strace -h`` 命令就知道这些
    参数的作用了，而以下的参数描述都是根据strace帮助文档整理的。

-  [-o file]：将strace的输出写入指定的文件。
-  [-s strsize]：将打印字符串的长度限制为strsize个字符（默认值为32）。
-  -d：输出strace关于标准错误的调试信息。
-  -r：打印出相对时间关于每一个系统调用。
-  -f：跟踪由fork()调用所产生的子进程。
-  -t：在输出中的每一行前加上时间信息。
-  -tt：在输出中的每一行前加上时间信息（微秒级）。
-  -T：打印在每个系统调用中花费的时间。
-  -r：打印相对时间戳。
-  -x：以十六进制打印非ascii字符串。
-  -xx：以十六进制打印所有字符串。
-  -y：打印与文件描述符参数关联的路径。
-  -yy：打印与套接字文件描述符关联的协议特定信息。

**strace命令统计参数**

-  -c：统计每一系统调用的所执行的时间，次数和出错的次数等。
-  -C：像-c一样，也可以打印常规输出。
-  -w：汇总系统调用延迟（默认为系统时间）
-  [-O overhead]：设置将系统调用跟踪到OVERUSE的开销
-  [-S sortby]：按以下顺序对系统调用计数：时间，调用，名称等

**strace命令过滤参数**

-  [-e
   expr]...：指定一个表达式，用来控制如何跟踪。格式如下： ``[qualifier=][!]value1[,value2]...`` 。

    描述说明： ``qualifier`` 只能是
    ``trace、abbrev、verbose、raw、signal、read、write`` 其中之一，value是用来限定的符号或数字，默认的
    qualifier是trace。感叹号是否定符号，例如: ``-e open`` 等价于
    ``-e trace=open`` ，表示只跟踪 ``open`` 调用；而 ``-etrace!=open``
    表示跟踪除了open以外的其他调用，还有两个特殊的符号 ``all`` 和
    ``none`` ，它们分别代表所有选项与没有选项。

**strace命令过滤的例子：**

-  ``-e trace=set``
   只跟踪指定的系统调用，例如: ``-e trace=open,close,rean,write`` 表示只跟踪这四个系统调用，默认的为 ``set=all`` 。
-  ``-e trace=file`` 只跟踪有关文件操作的系统调用。
-  ``-e trace=process`` 只跟踪有关进程控制的系统调用。
-  ``-e trace=network`` 跟踪与网络有关的所有系统调用。
-  ``-e strace=signal`` 跟踪所有与信号有关的系统调用。
-  ``-e trace=ipc`` 跟踪所有与进程通讯有关的系统调用。
-  ``-e raw=set`` 将指定的系统调用的参数以十六进制显示。
-  ``-e signal=se`` t
   指定跟踪的系统信号。默认为all（所有信号）。如 ``signal=!SIGIO`` 或者 ``signal=!io`` ，表示不跟踪 ``SIGIO`` 信号。
-  ``-e read=set`` 输出从指定文件中读出的数据。
-  ``-e write=set`` 输出写入到指定文件中的数据。

strace实战
----------

跟踪ls命令
~~~~~~~~~~

学习了那么多基本的知识，大家是不是迫不及待想试一下了？我们就跟踪一下基本的ls命令吧，它在我们linux系统
中应该是用得最多的命令了，我们可以看看在敲下ls命令的时候，系统做了什么东西：

    友情提示一下，因为ls命令是列出当前路径下的所有文件，而strace命令会默认跟踪所有的系统调用，如果当前路径下有很多文件的话，可能会输出非常非常多的内容，不利于观察，我们可以新建一个测试的空目录，在空目录下跟踪ls命令到底做了什么事情。

.. code:: bash

    # 创建空的测试目录并进入
    ➜  ~ mkdir mytest
    ➜  ~ cd mytest 

    # 跟踪ls命令
    ➜  mytest strace ls   

    # 输出内容
    execve("/bin/ls", ["ls"], 0x7fffe987f830 /* 34 vars */) = 0
    brk(NULL)                               = 0x55769774d000
    access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
    access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
    openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
    fstat(3, {st_mode=S_IFREG|0644, st_size=95106, ...}) = 0
    mmap(NULL, 95106, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7efd08f12000
    close(3)                                = 0
    access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
    openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libselinux.so.1", O_RDONLY|O_CLOEXEC) = 3
    read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\20b\0\0\0\0\0\0"..., 832) = 832
    fstat(3, {st_mode=S_IFREG|0644, st_size=154832, ...}) = 0
    mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7efd08f10000
    mmap(NULL, 2259152, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7efd08adb000
    mprotect(0x7efd08b00000, 2093056, PROT_NONE) = 0
    mmap(0x7efd08cff000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x24000) = 0x7efd08cff000
    mmap(0x7efd08d01000, 6352, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7efd08d01000

    # 省略n行

    close(3)                                = 0
    close(1)                                = 0
    close(2)                                = 0
    exit_group(0)                           = ?
    +++ exited with 0 +++

是不是没想到，这么简单的一个ls命令，内核居然敢了这么多事情，这还是省略了绝大部分的输出，大家可以自己参考终端的输出内容。

跟踪并统计ls命令
~~~~~~~~~~~~~~~~

或者我们可以统计一下ls命令调用了什么函数，可以使用以下命令：

.. code:: bash

    ➜  mytest strace -c ls           

    % time     seconds  usecs/call     calls    errors syscall
    ------ ----------- ----------- --------- --------- ----------------
      0.00    0.000000           0         7           read
      0.00    0.000000           0        11           close
      0.00    0.000000           0         9           fstat
      0.00    0.000000           0        17           mmap
      0.00    0.000000           0        12           mprotect
      0.00    0.000000           0         1           munmap
      0.00    0.000000           0         3           brk
      0.00    0.000000           0         2           rt_sigaction
      0.00    0.000000           0         1           rt_sigprocmask
      0.00    0.000000           0         2           ioctl
      0.00    0.000000           0         8         8 access
      0.00    0.000000           0         1           execve
      0.00    0.000000           0         2           getdents
      0.00    0.000000           0         2         2 statfs
      0.00    0.000000           0         1           arch_prctl
      0.00    0.000000           0         1           set_tid_address
      0.00    0.000000           0         9           openat
      0.00    0.000000           0         1           set_robust_list
      0.00    0.000000           0         1           prlimit64
    ------ ----------- ----------- --------- --------- ----------------
    100.00    0.000000                    91        10 total

这里的输出很清楚的告诉我们调用了那些系统函数，调用次数多少，消耗了多少时间等等，这些信息对我们分析一个程序来说是非常有用的。

重定向输出信息
~~~~~~~~~~~~~~

strace命令在终端输出的信息太多了，我们想要将它重定向输出某个文件中（使用 ``[-o filename]`` 参数），
然后对文件进行分析，这样子的操作就比在终端上分析要好得多，但是需要注意的是，因为输出的信息很多，
生成的日志文件可能会很大，所以在日常使用中要注意设置过滤，不需要完全跟踪所有的内容。

重定向输出信息命令如下：

.. code:: bash

    ➜  mytest strace -o ls.log ls 
    ls.log

    ➜  mytest strace -c -o ls.log ls      
    ls.log

生成ls.log文件位于当前路径下，可以使用cat命令打开。

    ps：-o filename
    参数中的文件名filename可以随意命名，无需后缀也是可以的，毕竟Linux一切皆文件。

跟踪自己的代码
~~~~~~~~~~~~~~

我们可以使用核心转储章节的代码，代码位置： ``embed_linux_tutorial/base_code/linux_debug/core_dump`` ，
编译后使用strace命令去跟踪它，看看遇到错误是怎么样的情况。

操作如下：

.. code:: bash

    # 编译
    ➜  core_dump git:(dev_jie) ✗ make 
    gcc -o core_dump.o -c -g -Werror -I. -Iinclude -static core_dump.c -g -MD -MF .core_dump.o.d
    gcc -o targets core_dump.o -g -Werror -I. -Iinclude -static

    # 跟踪
    ➜  core_dump git:(dev_jie) ✗ strace ./targets 
    execve("./targets", ["./targets"], 0x7ffe4c007260 /* 34 vars */) = 0
    brk(NULL)                               = 0x1d57000
    brk(0x1d581c0)                          = 0x1d581c0
    arch_prctl(ARCH_SET_FS, 0x1d57880)      = 0
    uname({sysname="Linux", nodename="embedfire_dev", ...}) = 0
    readlink("/proc/self/exe", "/home/xxxxx/embed_linux_tutoria"..., 4096) = 73
    brk(0x1d791c0)                          = 0x1d791c0
    brk(0x1d7a000)                          = 0x1d7a000
    access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
    fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(136, 0), ...}) = 0
    write(1, "\350\277\231\346\230\257\344\270\200\344\270\252\351\224\231\350\257\257\n", 19这是一个错误
    ) = 19
    --- SIGSEGV {si_signo=SIGSEGV, si_code=SEGV_MAPERR, si_addr=NULL} ---
    +++ killed by SIGSEGV +++
    [1]    12576 segmentation fault  strace ./targets

从日志信息可以看出，它是被SIGSEGV终止的，并且产生了段错误 ``segmentation fault`` 。除此之外，
代码的运行调用了一些系统调用，比如调用execve()函数创建了一个新的进程 ``./targets`` 、查看了当前系统的信息，
系统名，主机名、打开文件描述符编号为1的输出文件，并且向该文件描述符中写入打印的内容等。

我们也能使用 ``-T`` 参数查看每个系统调用的时间（在输出的最右边 ``<>`` 的内容就是时间），当然大家可以用 ``-t -tt`` 等参数来测试一下：

.. code:: bash

    ➜  core_dump git:(dev_jie) ✗ strace -T ./targets

    execve("./targets", ["./targets"], 0x7ffc6c9cda78 /* 34 vars */) = 0 <0.000119>
    brk(NULL)                               = 0x1d65000 <0.000006>
    brk(0x1d661c0)                          = 0x1d661c0 <0.000007>
    arch_prctl(ARCH_SET_FS, 0x1d65880)      = 0 <0.000005>
    uname({sysname="Linux", nodename="embedfire_dev", ...}) = 0 <0.000005>
    readlink("/proc/self/exe", "/home/jiejie/embed_linux_tutoria"..., 4096) = 73 <0.000023>
    # 省略后续输出

本章跟踪进程的讲解就到此结束，更多实战的内容大家可以自行去尝试。
