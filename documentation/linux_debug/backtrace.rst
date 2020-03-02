gdb调试之函数调用栈——backtrace
==============================

什么是函数调用栈
----------------

在写代码的时候，我们会封装很多函数，而这些函数之中又会调用其他的函数，当程序每次调用函数的时候，就会跳转到函数的地方去执行，那么这期间就有很多信息产生了，比如：调用函数的地方，函数的参数，被调用函数的变量等，这些信息其实是存储在栈中的，其实更确切地说，这些信息是存储在函数调用信息帧中的，每个函数及其变量都被分配了一个帧（frame），这些函数信息帧就组成了函数调用栈。我们使用
gdb
调试工具就可以查看函数调用栈的内容信息，可以清晰地看到各个函数的调用顺序以及各函数的输入形参值，是分析程序的执行流程和输入依赖的重要手段。

gdb提供了一些指令可以查看这些帧中的信息，当查询函数变量的信息时，gdb就是从这个被选中的帧内获取信息，但是查看被选中帧外的变量信息是非法的，当程序运行停止的时候，gdb会自动选择当前被调用的函数帧，并且打印简单帧信息。

gdb中函数调用栈的指令
---------------------

查看栈信息
~~~~~~~~~~

-  ``bt``\ ：bt是\ ``backtrace``\ 指令的缩写，显示所有的函数调用栈的信息，栈中的每个函数都被分配了一个编号，最近被调用的函数在
   0 号帧中（栈顶），并且每个帧占用一行。

-  ``bt n``\ ：显示函数调用栈从栈顶算起的n帧信息（n 表示一个正整数）。

-  ``bt -n``\ ：显示函数调用栈从栈底算起的n帧信息。

-  ``bt full``\ ：显示栈中所有信息如：函数参数，本地变量等。

-  ``bt full n``\ ：显示函数调用栈从栈顶算起的n帧的所有信息。

-  ``bt full -n``\ ：显示函数调用栈从栈底算起的n帧的所有信息。

查看帧信息
~~~~~~~~~~

上面的bt指令主要是查看栈的信息，而每一帧都会有详细的信息，这些函数调用信息帧包括：调用函数的地方，函数的参数等。如果想查看栈中某一帧的信息，首先要做的是切换当前栈。这时候需用用到
frame 指令（缩写形式为 f）。

-  ``f n``: 它的功能是切换到编号为 n 的栈帧（n
   表示一个正整数），并显示相关信息。

up/down 指令
~~~~~~~~~~~~

除了使用 frame 指令切换栈帧外，还可以使用 up 和 down 指令。

-  ``down n``\ ：表示往栈顶方向下移 n 层（n 表示一个正整数，默认值为
   1）。
-  ``up n``\ ：表示往栈底方向上移 n 层。

查看更详细的帧信息
~~~~~~~~~~~~~~~~~~

info
指令是一个很强大的指令，使用它可以查看各种变量的值，如果我们希望看到详细的函数调用信息帧的信息，如：函数地址、调用函数的地址、被调用函数的地址、当前函数由哪种编程语言编写、函数参数地址及形参值、局部变量的地址、当前桢中存储的寄存器等，可以使用以下指令：

-  ``info frame``\ ：
   指令的缩写形式为\ ``i f``\ ，查看函数调用帧的所有信息。

-  ``info args``\ ：查看函数变量的值。

-  ``info locals``\ ：查看本地变量的信息。

除此之外 info 指令还可以查看当前寄存器的值：

-  ``info registers``\ ：查看寄存器的情况（除了浮点寄存器）。

-  ``info all-registers``\ ：查看所有寄存器的情况（包括浮点寄存器）。

gdb调试实战
-----------

调试源码
~~~~~~~~

首先我们得写一份可以调试函数调用栈的代码，也就是说一个函数要调用其他函数，层层调用，这样子才能在调试中看得出来调用栈相关的信息，代码如下：

.. code:: c

    #include <sys/types.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <stdlib.h>

    void test1(int arg)
    {
        int num;
        num = arg;
        
        printf("\t\t---我是 test%d 函数\n", num);
    }

    void test2(int arg)
    {
        int num;
        num = arg;

        printf("\t--我是 test%d 函数\n", num);

        printf("\t-- test%d 开始调用 test1 \n", num);
        test1(1);
        printf("\t-- test%d 结束调用 test1 \n", num);

        printf("\t--结束调用 test%d \n", num);
    }

    void test3(int arg)
    {
        int num;
        num = arg;

        printf("-我是 test%d 函数\n", num);

        printf("- test%d 开始调用 test2 \n", num);
        test2(2);
        printf("- test%d 结束调用 test1 \n", num);

        printf("-结束调用 test%d \n", num);
    }

    int main(void)
    {
        test3(3);

        sleep(1);       // 防止进程过快退出

        return 0;
    }

代码呢非常简单，函数的调用关系如下:

.. code:: bash

    main()
        ->test3()
            ->test2()
                ->test1()

然后我们将源码编译，然后使用gdb调试器进行调试，我们在实际调试的时候，先去到最底层的地方，即test1()函数中，查看它被调用的关系，具体操作如下：

1. 编译源码，并尝试运行它，可以很明显看到函数的调用关系信息：

.. code:: bash

    ➜  backtrace git:(master) ✗ make
    gcc -o backtrace.o -c -g -Werror -I. -Iinclude -static backtrace.c -g -MD -MF .backtrace.o.d
    gcc -o targets backtrace.o -g -Werror -I. -Iinclude -static

    # 运行
    ➜  backtrace git:(master) ✗ ./targets 
    -我是 test3 函数
    - test3 开始调用 test2 
            --我是 test2 函数
            -- test2 开始调用 test1 
                    ---我是 test1 函数
            -- test2 结束调用 test1 
            --结束调用 test2 
    - test3 结束调用 test1 
    -结束调用 test3 

2. 使用gdb调试器进行调试：

.. code:: bash

    ➜  backtrace git:(master) ✗ gdb ./targets 

    ···
    Reading symbols from ./targets...done.
    (gdb) 

3. 在test1()函数中打上断点，并运行到断点处，代码运行期间会打印出相关的信息：

.. code:: bash

    # 打断点
    (gdb) b test1
    Breakpoint 1 at 0x400b58: file backtrace.c, line 9.

    # 运行到断点处
    (gdb) r
    Starting program: /home/jiejie/embed_linux_tutorial/base_code/linux_debug/backtrace/targets 

    # 这些就是代码运行时打印的信息
    -我是 test3 函数
    - test3 开始调用 test2 
            --我是 test2 函数
            -- test2 开始调用 test1 

    Breakpoint 1, test1 (arg=1) at backtrace.c:9
    9           num = arg;

4. bt指令查看函数调用栈：

可以看到一些基本的信息，如函数调用信息帧的地址，传递的参数arg的值，以及代码的位置等信息。

.. code:: bash

    (gdb) bt
    #0  test1 (arg=1) at backtrace.c:9
    #1  0x0000000000400bbe in test2 (arg=2) at backtrace.c:22
    #2  0x0000000000400c34 in test3 (arg=3) at backtrace.c:36
    #3  0x0000000000400c71 in main () at backtrace.c:44

    (gdb) bt 2
    #0  test1 (arg=1) at backtrace.c:9
    #1  0x0000000000400bbe in test2 (arg=2) at backtrace.c:22
    (More stack frames follow...)

    (gdb) bt -2
    #2  0x0000000000400c34 in test3 (arg=3) at backtrace.c:36
    #3  0x0000000000400c71 in main () at backtrace.c:44

5. f指令切换帧：

.. code:: bash

    (gdb) f 2
    #2  0x0000000000400c34 in test3 (arg=3) at backtrace.c:36
    36          test2(2);

    (gdb) f 3
    #3  0x0000000000400c71 in main () at backtrace.c:44
    44          test3(3);

6. ``info frame``\ 指令查看函数调用帧的所有信息，在这步操作前我们切换回到test1函数中（切换帧）。

.. code:: bash

    # 切换帧 0
    (gdb) f 0
    #0  test1 (arg=1) at backtrace.c:9
    9           num = arg;

    # 查看信息
    (gdb) i f
    Stack level 0, frame at 0x7fffffffe030:
     rip = 0x400b58 in test1 (backtrace.c:9); saved rip = 0x400bbe
     called by frame at 0x7fffffffe060
     source language c.
     Arglist at 0x7fffffffe020, args: arg=1
     Locals at 0x7fffffffe020, Previous frame's sp is 0x7fffffffe030
     Saved registers:
      rbp at 0x7fffffffe020, rip at 0x7fffffffe028

这里面有很多信息：

-  当前桢的地址：0x7fffffffe030。

-  rip的值：0x400b58，此处引申介绍一下rip是什么：它是指令地址寄存器，用来存储
   CPU 即将要执行的指令地址。每次 CPU 执行完相应的汇编指令之后，rip
   寄存器的值就会自行累加，rip 无法直接赋值。

-  当前桢函数：test1 (backtrace.c:9)。

-  调用者的rip值：saved rip = 0x400bbe。

-  调用者的帧地址：0x7fffffffe060。

-  源代码所用的程序的语言: source language c。

-  当前桢的参数的地址及值：Arglist at 0x7fffffffe020, args: arg=1。

-  当前帧中局部变量的地址：Locals at 0x7fffffffe020, Previous frame's sp
   is 0x7fffffffe030。

-  当前桢中存储的寄存器：rbp at 0x7fffffffe020, rip at 0x7fffffffe028。

7. ``info args``\ 指令查看函数变量的值。

.. code:: bash

    (gdb) info args 
    arg = 1

8. ``info locals``\ 指令查看本地变量的信息，如果此时还未运行变量赋值语句，则变量不会有值。

.. code:: bash

    Breakpoint 1, test1 (arg=1) at backtrace.c:9
    9           num = arg;
    (gdb) s
    11          printf("\t\t---我是 test%d 函数\n", num);
    (gdb) info locals
    num = 1

1. 查看当前寄存器的值（不包括浮点寄存器）：

.. code:: bash

    (gdb) info registers
    rax            0x1      1
    rbx            0x400400 4195328
    rcx            0x0      0
    rdx            0x6bbd30 7060784
    rsi            0x0      0
    rdi            0x1      1
    rbp            0x7fffffffe020   0x7fffffffe020
    rsp            0x7fffffffe000   0x7fffffffe000
    r8             0x0      0
    r9             0x1e     30
    r10            0x0      0
    r11            0x246    582
    r12            0x401a00 4200960
    r13            0x0      0
    r14            0x6b9018 7049240
    r15            0x0      0
    rip            0x400b5e 0x400b5e <test1+17>
    eflags         0x206    [ PF IF ]
    cs             0x33     51
    ss             0x2b     43
    ds             0x0      0
    es             0x0      0
    fs             0x0      0
    gs             0x0      0

gdb调试递归函数
---------------

本小节的主题是教大家用gdb去调试递归函数，因为一步步去调试太麻烦了，也没法打断点，因为打断点每次递归时都会停下来，实在是难以调试，那么强大如gdb调试工具，对这种递归函数的调试也是轻而易举的。

我们用递归算法计算斐波拉契数列，这是在大学C语言课本中的非常有名的递归算法——计算斐波拉契数列，我们回顾一下是什么是斐波拉契数列：斐波那契数列由
0 和 1 开始，之后的斐波那契数就是由之前的两数相加而得出。

递归算法计算就非常简单啦，代码如下：

.. code:: c

    int fibonacci(int n)
    {       
        if (n == 1 || n == 2) {   
            return 1;
        }   

        return fibonacci(n - 1) + fibonacci(n - 2);                                                                                                            
    }       
            
    int main()
    {       
        int n = 10; 
        int ret = 0;

        ret = fibonacci(n);

        printf("fibonacci(%d)=%d\n", n, ret);

        return 0;
    }

编译后使用gdb调试，比如我打算当递归函数fibonacci()中 n
的值为5时，停下来，然后查看函数的调用栈：

.. code:: bash

    # 启动gdb调试
    ➜  backtrace git:(master) ✗ gdb ./targets 

    # 打断点，当n等于5时在断点处停下
    (gdb) b fibonacci if n==5
    Breakpoint 1 at 0x400b59: file backtrace.c, line 54.

    # 运行，停下来时n已经等于5了
    (gdb) r
    Starting program: /home/jiejie/embed_linux_tutorial/base_code/linux_debug/backtrace/targets 

    Breakpoint 1, fibonacci (n=5) at backtrace.c:54
    54          if (n == 1 || n == 2) { 

    # 查看函数调用栈
    (gdb) bt
    #0  fibonacci (n=5) at backtrace.c:54
    #1  0x0000000000400b79 in fibonacci (n=6) at backtrace.c:58
    #2  0x0000000000400b79 in fibonacci (n=7) at backtrace.c:58
    #3  0x0000000000400b79 in fibonacci (n=8) at backtrace.c:58
    #4  0x0000000000400b79 in fibonacci (n=9) at backtrace.c:58
    #5  0x0000000000400b79 in fibonacci (n=10) at backtrace.c:58
    #6  0x0000000000400bb1 in main () at backtrace.c:66

    # 其他的一些操作...

至此，本章内容讲解完毕，更多的gdb调试信息大家可以亲身去体验。
