消息队列
========

Linux下的进程通信手段基本上是从Unix平台上的进程通信手段继承而来的。而对Unix发展做出重大贡献的两大主力AT&T的贝尔实验室及BSD（加州大学伯克利分校的伯克利软件发布中心）在进程间通信方面的侧重点有所不同；前者对Unix早期的进程间通信手段进行了系统的改进和扩充，形成了"system
V
IPC"，通信进程局限在单个计算机内（同一个设备的不同进程间通讯）；而后者则跳过了该限制，形成了基于套接字（socket）的进程间通信机制（多用于不同设备的进程间通讯）。Linux则把两者继承了下来，所以说Linux才是最成功的，既有"system
V IPC"，又支持socket。

**消息队列、共享内存 和 信号量**\ 被统称为 system-V IPC，V 是罗马数字
5，是 Unix 的AT&T 分支的其中一个版本，一般习惯称呼他们为 IPC
对象，这些对象的操作接口都比较类似，在系统中他们都使用一种叫做 key
的键值来唯一标识，而且他们都是"持续性"资源——即他们被创建之后，不会因为进程的退出而消失，而会持续地存在，除非调用特殊的函数或者命令删除他们。

消息队列的基本概念
------------------

Linux的消息队列（queue）实质上是一个内核地址空间中的内部链表，它有消息队列标识符（queue
ID）。通过Linux内核在各个进程直接传递内容，消息顺序地发送到消息队列中，并以几种不同的方式从队列中获得，每个消息队列可以用消息队列标识符唯一地进行识别。内核中是通过IPC的标识符来区别不同的消息队列，不同的消息队列之间是相互独立的，每个消息队列中的消息，又构成一个独立的链表，这样子就是在内涵中维护多个消息队列，每个消息队列的内部又通过链表维护了不同的消息，举个不太恰当的比喻：假设你的房子就是内核，那么每个房间就是消息队列，一个房子有多个房间，而每个房间睡觉的人就是不同的消息，有一些房间可以睡很多个人，有一些房间没有人。在Linux系统中，只要有足够权限的进程都可以向某个消息队列中发送消息，被赋予读权限的进程则可以读走这个消息队列中的消息，

当然也可能有同学对消息队列的标识符感兴趣，那我就简单介绍一下：每个内核中的
IPC
结构（消息队列、信号量或共享存储段）都用一个非负整数的标识符（identifier）加以引用。例如，为了对一个消息队列发送或取消息，只需知道其消息队列标识符。这个消息队列标识符是在内部使用，因此这样的标识方法不能支持进程间通信的，那怎么办呢？别急，且听我慢慢道来!

而我们用户接触到的只是IPC关键字，并非IPC标识符，这是因为IPC标识符是IPC结构的内部名。为使多个合作进程能够在同一IPC对象上会合，需要提供一个外部名方案，即关键字（key）每一个IPC对象都与一个IPC关键字相关联，于是关键字就作为该IPC结构的外部名。要想获得一个唯一标识符，必须使用一个IPC关键字，这样子只要不同进程间使用的关键字是相同的，就可以得到相同的IPC标识符，这样子就能保证访问到相同的消息队列。因此无论何时创建IPC结构，都应指定一个关键字（key），关键字的数据类型由系统规定为
key\_t，是一个长整型的数据类型，关键字由内核变换成标识符。

**消息队列与信号的对比：**

-  信号承载的信息量少，而消息队列可以承载大量的数据

**消息队列与管道的对比：**

-  消息队列跟命名管道有不少的相同之处，通过与命名管道一样，消息队列进行通信的进程可以是不相关的进程，同时它们都是通过发送和接收的方式来传递数据的。在命名管道中，发送数据用write()，接收数据用read()，则在消息队列中，发送数据用msgsnd()，接收数据用msgrcv()。它们对每个数据都有一个最大长度的限制。

-  消息队列也可以独立于发送和接收进程而存在，在进程终止时，消息队列及其内容并不会被删除。

-  管道只能承载无格式字节流，消息队列提供有格式的字节流，可以减少了开发人员的工作量。

-  消息队列是面向记录的，其中的消息具有特定的格式以及特定的优先级，接收程序可以通过消息类型有选择地接收数据，而不是像命名管道中那样，只能默认地接收。

-  消息队列可以实现消息的随机查询，消息不一定要以先进先出的顺序接收，也可以按消息的类型接收。

消息队列的实现包括创建或打开消息队列、发送消息、接收消息和控制消息队列这
4 种操作。其中创建或打开消息队列使用的函数是
msgget()，这里创建的消息队列的数量会受到系统可支持的消息队列数量的限制；发送消息使用的函数是
msgsnd()函数，它把消息发送到已打开的消息队列末尾;接收消息使用的函数是msgrcv()，它把消息从消息队列中取走，与
FIFO 不同的是，这里可以指定取走某一条消息;最后控制消息队列使用的函数是
msgctl()，它可以完成多项功能。

想要使用消息队列还需要包含以下头文件：

.. code:: c

        #include <sys/types.h>
        #include <sys/ipc.h>
        #include <sys/msg.h>

创建或获取消息队列ID
--------------------

**msgget()函数**

.. code:: c

       int msgget(key_t key, int msgflg);

该函数的作用是得到消息队列标识符或创建一个消息队列对象并返回消息队列标识符：成功返回队列ID，失败返回-1。为什么说这个函数是创建或者获取消息队列ID呢？在以下两种情况下，msgget()函数将创建一个新的消息队列：

1. 如果没有与键值key相对应的消息队列，并且flag中包含了IPC\_CREAT标志位。
2. key参数为IPC\_PRIVATE。

-  key:消息队列的关键字值，多个进程可以通过它访问同一个消息队列，其中有个特殊值
   IPC\_PRIVATE，它用于创建当前进程的私有消息队列。

-  msgflg表示创建的消息队列的模式标志参数，在真正使用时需要与IPC对象存取权限（如0600）进行"｜"运算来确定消息队列的存取权限。msgflg有多种情况：如果为0则表示取消息队列标识符，若该消息队列不存在则函数会报错；如果是\ ``IPC_CREAT & msgflg``
   为真则表示：如果内核中不存在关键字与key相等的消息队列，则新建一个消息队列；如果存在这样的消息队列，返回此消息队列的标识符，而如果为\ ``IPC_CREAT | IPC_EXCL``\ 为真则表示：如果内核中不存在键值与key相等的消息队列，则新建一个消息队列；如果存在这样的消息队列则报错，这些参数是可以通过"｜"运算符联合起来的，因为它始终是int类型的参数。

该函数可能返回以下错误代码：

-  EACCES：指定的消息队列已存在，但调用进程没有权限访问它

-  EEXIST：key指定的消息队列已存在，而msgflg中同时指定IPC\_CREAT和IPC\_EXCL标志

-  ENOENT：key指定的消息队列不存在同时msgflg中没有指定IPC\_CREAT标志

-  ENOMEM：需要建立消息队列，但内存不足

-  ENOSPC：需要建立消息队列，但已达到系统的限制

**使用该函数需要注意的以下几点:**

1. 选项 msgflg 是一个位屏蔽字，因此 IPC\_CREAT、IPC\_EXCL 和权限 mode
   可以用位或的方式叠加起来，比如:\ ``msgget(key, IPC_CREAT | 0666);``\ 表示如果
   key 对应的消息队列不存在就创建，且权限指定为
   0666，若已存在则直接获取消息队列ID。
2. 权限只有读和写，执行权限是无效的，例如 0777 跟 0666 是等价的。
3. 当 key 被指定为 IPC\_PRIVATE 时，系统会自动产生一个未用的 key
   来对应一个新的消息队列对象，这个消息队列一般用于进程内部间的通信。

发送消息与接收消息
------------------

**msgsnd()函数**

这个函数的主要作用就是将消息写入到消息队列，俗称发送一个消息。

**函数原型：**

.. code:: c

        int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

**函数传入值：**

-  msqid：消息队列标识符。

-  msgp：发送给队列的消息。msgp可以是任何类型的结构体，但第一个字段必须为long类型，即表明此发送消息的类型，msgrcv()函数则根据此接收消息。msgp定义的参照格式如下：

   .. code:: c

           /*msgp定义的参照格式*/
           struct s_msg{ 
               long type;  /* 必须大于0,消息类型 */
               char mtext[１];  /* 消息正文，可以是其他任何类型 */
           } msgp;

-  msgsz：要发送消息的大小，不包含消息类型占用的4个字节，即mtext的长度。

-  msgflg：如果为0则表示：当消息队列满时，msgsnd()函数将会阻塞，直到消息能写进消息队列；如果为IPC\_NOWAIT则表示：当消息队列已满的时候，msgsnd()函数不等待立即返回；如果为IPC\_NOERROR：若发送的消息大于size字节，则把该消息截断，截断部分将被丢弃，且不通知发送进程。

如果成功则返回0，如果失败则返回-1，并且错误原因存于error中。

**错误代码：**

-  EAGAIN：参数msgflg设为IPC\_NOWAIT，而消息队列已满。

-  EIDRM：标识符为msqid的消息队列已被删除。

-  EACCESS：无权限写入消息队列。

-  EFAULT：参数msgp指向无效的内存地址。

-  EINTR：队列已满而处于等待情况下被信号中断。

-  EINVAL：无效的参数msqid、msgsz或参数消息类型type小于0。

msgsnd()为阻塞函数，当消息队列容量满或消息个数满会阻塞。消息队列已被删除，则返回EIDRM错误；被信号中断返回E\_INTR错误。

如果设置IPC\_NOWAIT消息队列满或个数满时会返回-1，并且置EAGAIN错误。

msgsnd()解除阻塞的条件有以下三个条件：

1. 消息队列中有容纳该消息的空间。
2. msqid代表的消息队列被删除。
3. 调用msgsnd函数的进程被信号中断。

**msgrcv()函数**

函数原型：

.. code:: c

        ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);

msgrcv()函数是从标识符为msqid的消息队列读取消息并将消息存储到msgp中，读取后把此消息从消息队列中删除，也就是俗话说的接收消息。

**函数传入值：**

-  msqid：消息队列标识符。

-  msgp：存放消息的结构体，结构体类型要与msgsnd()函数发送的类型相同。

-  msgsz：要接收消息的大小，不包含消息类型占用的4个字节。

-  msgtyp有多个可选的值：如果为0则表示接收第一个消息，如果大于0则表示接收类型等于msgtyp的第一个消息，而如果小于0则表示接收类型等于或者小于msgtyp绝对值的第一个消息。

**msgflg取值情况如下：**

-  0: 阻塞式接收消息，没有该类型的消息msgrcv函数一直阻塞等待

-  IPC\_NOWAIT：若在消息队列中并没有相应类型的消息可以接收，则函数立即返回，此时错误码为ENOMSG

-  IPC\_EXCEPT：与msgtype配合使用返回队列中第一个类型不为msgtype的消息

-  IPC\_NOERROR：如果队列中满足条件的消息内容大于所请求的size字节，则把该消息截断，截断部分将被丢弃

msgrcv()函数如果接收消息成功则返回实际读取到的消息数据长度，否则返回-1，错误原因存于error中。

**错误代码**

-  E2BIG：消息数据长度大于msgsz而msgflag没有设置IPC\_NOERROR

-  EIDRM：标识符为msqid的消息队列已被删除

-  EACCESS：无权限读取该消息队列

-  EFAULT：参数msgp指向无效的内存地址

-  ENOMSG：参数msgflg设为IPC\_NOWAIT，而消息队列中无消息可读

-  EINTR：等待读取队列内的消息情况下被信号中断

msgrcv()函数解除阻塞的条件也有三个：

1. 消息队列中有了满足条件的消息。
2. msqid代表的消息队列被删除。
3. 调用msgrcv()函数的进程被信号中断。

操作消息队列
------------

消息队列是可以被用户操作的，比如设置或者获取消息队列的相关属性，那么可以通过msgctl()函数去处理它。

**函数原型：**

.. code:: c

    int msgctl(int msqid, int cmd, struct msqid_ds *buf);

**函数传入值：**

-  msqid：消息队列标识符。

**cmd的取值有多个：**

-  IPC\_STAT 获取该 MSG 的信息，获取到的信息会储存在结构体 msqid\_ds
   类型的buf中。

-  IPC\_SET 设置消息队列的属性，要设置的属性需先存储在结构体
   msqid\_ds类型的buf中，可设置的属性包括：msg\_perm.uid、msg\_perm.gid、msg\_perm.mode以及msg\_qbytes，储存在结构体
   msqid\_ds。

-  IPC\_RMID 立即删除该 MSG，并且唤醒所有阻塞在该 MSG
   上的进程，同时忽略第三个参数。

-  IPC\_INFO 获得关于当前系统中 MSG 的限制值信息。

-  MSG\_INFO 获得关于当前系统中 MSG 的相关资源消耗信息。

-  MSG\_STAT 同 IPC\_STAT，但 msgid
   为该消息队列在内核中记录所有消息队列信息的数组的下标，因此通过迭代所有的下标可以获得系统中所有消息队列的相关信息。

-  buf：相关信息结构体缓冲区。

**函数返回值：**

-  成功：0

-  出错：-1，错误原因存于error中

**错误代码：**

-  EACCESS：参数cmd为IPC\_STAT，确无权限读取该消息队列。

-  EFAULT：参数buf指向无效的内存地址。

-  EIDRM：标识符为msqid的消息队列已被删除。

-  EINVAL：无效的参数cmd或msqid。

-  EPERM：参数cmd为IPC\_SET或IPC\_RMID，却无足够的权限执行。

消息队列实例
------------

消息队列的使用方法一般是:

**发送者:**

1. 获取消息队列的 ID
2. 将数据放入一个附带有标识的特殊的结构体，发送给消息队列。

**接收者:**

1. 获取消息队列的 ID
2. 将指定标识的消息读出。

当发送者和接收者都不再使用消息队列时，及时删除它以释放系统资源。

本次实验主要是两个进程（无血缘关系的进程）通过消息队列进行消息的传递，一个进程发送消息，一个进程接收消息，并将其打印出来。

**发送进程源码**

.. code:: c

    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/msg.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <string.h>


    #define BUFFER_SIZE 512

    struct message
    {
        long msg_type;
        char msg_text[BUFFER_SIZE];
    };
    int main()
    {
        int qid;
        struct message msg;

        /*创建消息队列*/
        if ((qid = msgget((key_t)1234, IPC_CREAT|0666)) == -1)
        {
            perror("msgget");
            exit(1);
        }

        printf("Open queue %d\n",qid);

        while(1)
        {
            printf("Enter some message to the queue:");
            if ((fgets(msg.msg_text, BUFFER_SIZE, stdin)) == NULL)
            {
                puts("no message");
                exit(1);
            }
            
            msg.msg_type = getpid();

            /*添加消息到消息队列*/
            if ((msgsnd(qid, &msg, strlen(msg.msg_text), 0)) < 0)
            {
                perror("message posted");
                exit(1);
            }

            if (strncmp(msg.msg_text, "quit", 4) == 0)
            {
                break;
            }
        }

        exit(0);
    }

**接收进程源码：**

.. code:: c

    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/msg.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <string.h>

    #define BUFFER_SIZE 512

    struct message
    {
        long msg_type;
        char msg_text[BUFFER_SIZE];
    };

    int main()
    {
        int qid;
        struct message msg;

        /*创建消息队列*/
        if ((qid = msgget((key_t)1234, IPC_CREAT|0666)) == -1)
        {
            perror("msgget");
            exit(1);
        }

        printf("Open queue %d\n", qid);

        do
        {
            /*读取消息队列*/
            memset(msg.msg_text, 0, BUFFER_SIZE);

            if (msgrcv(qid, (void*)&msg, BUFFER_SIZE, 0, 0) < 0)
            {
                perror("msgrcv");
                exit(1);
            }

            printf("The message from process %ld : %s", msg.msg_type, msg.msg_text);

        } while(strncmp(msg.msg_text, "quit", 4));

        /*从系统内核中移走消息队列 */
        if ((msgctl(qid, IPC_RMID, NULL)) < 0)
        {
            perror("msgctl");
            exit(1);
        }

        exit(0);

    }

将两个进程编译出来，分别运行即可，实验现象如下：

**发送消息：**

在发送消息进程运行的时候，会提示让你输入要发送的消息，随便什么消息都可以的

.. code:: bash

    ➜  msg_send ./targets

    Open queue 0
    Enter some message to the queue:123
    Enter some message to the queue:1111111 
    Enter some message to the queue:666666666666666
    Enter some message to the queue:abcdef
    Enter some message to the queue:

**接收消息：**

在新的终端运行接收消息进程，当你从发送消息进程输入消息时（按下回车键发送），接收消息进程会打印出你输入的消息，这是因为进程从消息队列中读取到发送进程发送的消息。

.. code:: bash

    ➜  msg_recv ./targets

    Open queue 0
    The message from process 12822 : 123
    The message from process 12822 : 1111111
    The message from process 12822 : 666666666666666
    The message from process 12822 : abcdef

