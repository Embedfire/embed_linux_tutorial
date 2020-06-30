套接字
======

不知道大家还记不记得我在消息队列章节中提到的一段话：

    Linux下的进程通信手段基本上是从Unix平台上的进程通信手段继承而来的。而对Unix发展做出重大贡献的两大主力AT&T的贝尔实验室及BSD（加州大学伯克利分校的伯克利软件发布中心）在进程间通信方面的侧重点有所不同；前者对Unix早期的进程间通信手段进行了系统的改进和扩充，形成了"system
    V
    IPC“，通信进程局限在单个计算机内（同一个设备的不同进程间通讯）；而后者则跳过了该限制，形成了基于套接字（Socket）的进程间通信机制（多用于不同设备的进程间通讯）。Linux则把两者继承了下来，所以说Linux才是最成功的，既有”system
    V IPC"，又支持Socket。

那么很显然，我们前面的文章讲解了单个计算机内的进程间通信，所有的机制都依靠一台计算机系统的共享资源实现，这里的资源可以是文件系统空间、共享的物理内存或消息队列，但只有运行在同一台机器上的进程才能使用它们，很显然这种方式并不满足我们想要的，我们可能会想让这个设备与另一个设备进行通讯，我接下来就要讲解用于不同设备的进程间通讯——基于套接字（Socket）的进程间通信机制，当然Socket也能支持在一个设备上的进程间通信，而不仅仅只是不同设备之间的。

在上一节我们也简单了解了网络中的UDP与TCP协议，那么我们今天就是通过套接字进行通讯。

Socket简介
----------

套接字（socket）是一种通信机制，凭借这种机制， ``客户端<->服务器``
模型的通信方式既可以在本地设备上进行，也可以跨网络进行。套接字的创建和使用与管道是有区别的，因为套接字明确地将客户端、服务器区分开来，而且套接字机制可以实现将多个客户连接到一个服务器。

Socket英文原意是“孔”或者“插座”的意思，在网络编程中，通常将其称之为“套接字”，当前网络中的主流程序设计都是使用Socket进行编程的，因为它简单易用，它还是一个标准（BSD
Socket），能在不同平台很方便移植，比如你的一个应用程序是基于Socket编程的，那么它可以移植到任何实现BSD
Socket标准的平台，比如LwIP，它兼容BSD
Socket；又比如Windows，它也实现了一套基于Socket的套接字接口，更甚至在国产操作系统中，如RT-Thread，它也实现了BSD
Socket标准的Socket接口。

在Socket中，它使用一个套接字来记录网络的一个连接，套接字是一个整数，就像我们操作文件一样，利用一个文件描述符，可以对它打开、读、写、关闭等操作，类似的，在网络中，我们也可以对Socket套接字进行这样子的操作，比如开启一个网络的连接、读取连接主机发送来的数据、向连接的主机发送数据、终止连接等操作。

我们来了解一下套接字描述符，它跟我们的文件描述符非常像，其实就是一个整数，套接字API最初是作为UNIX操作系统的一部分而开发的，所以套接字API与系统的其他I/O设备集成在一起。当应用程序要为网络通信而创建一个套接字（socket）时，操作系统就返回一个整数作为描述符（descriptor）来标识这个套接字。然后，应用程序以该描述符作为传递参数，通过调用Socket
API接口的函数来完成某种操作（例如通过网络传送数据或接收输入的数据）。

接下来讲解Linux系统中的套接字相关的函数，但注意要包含网络编程中常用的头文件：

.. code:: c

        #include <sys/types.h>
        #include <sys/socket.h>

socket()
--------

函数原型：

.. code:: c

        int socket(int domain, int type, int protocol);

socket()函数用于创建一个socket描述符（socket
descriptor），它唯一标识一个socket，这个socket描述字跟文件描述字一样，后续的操作都有用到它，把它作为参数，通过它来进行一些读写操作。

创建socket的时候，也可以指定不同的参数创建不同的socket描述符，socket函数的三个参数分别为：
1.
domain：参数domain表示该套接字使用的协议族，在Linux系统中支持多种协议族，对于TCP/IP协议来说，选择AF_INET就足以，当然如果你的IP协议的版本支持IPv6，那么可以选择AF_INET6，可选的协议族具体见：

-  AF_UNIX, AF_LOCAL： 本地通信
-  AF_INET ： IPv4
-  AF_INET6 ： IPv6
-  AF_IPX ： IPX - Novell 协议
-  AF_NETLINK ： 内核用户界面设备
-  AF_X25 ： ITU-T X.25 / ISO-8208 协议
-  AF_AX25 ： 业余无线电 AX.25 协议
-  AF_ATMPVC ： 访问原始ATM PVC
-  AF_APPLETALK ： AppleTalk
-  AF_PACKET ： 底层数据包接口
-  AF_ALG ： 内核加密API的AF_ALG接口

    ps：其实我个人都只用过AF_INET以及AF_NETLINK，对于其他协议族，基本没用过。

1. type：参数type指定了套接字使用的服务类型，可能的类型有以下几种：

-  SOCK_STREAM：提供可靠的（即能保证数据正确传送到对方）面向连接的Socket服务，多用于资料（如文件）传输，如TCP协议。
-  SOCK_DGRAM：是提供无保障的面向消息的Socket
   服务，主要用于在网络上发广播信息，如UDP协议，提供无连接不可靠的数据报交付服务。
-  SOCK_SEQPACKET：为固定最大长度的数据报提供有序的，可靠的，基于双向连接的数据传输路径。
-  SOCK_RAW：表示原始套接字，它允许应用程序访问网络层的原始数据包，这个套接字用得比较少，暂时不用理会它。
-  SOCK_RDM：提供不保证排序的可靠数据报层。

2. protocol：参数protocol指定了套接字使用的协议，在IPv4中，只有TCP协议提供SOCK_STREAM这种可靠的服务，只有UDP协议提供SOCK_DGRAM服务，对于这两种协议，protocol的值均为0，因为当protocol为0时，会自动选择type类型对应的默认协议。。

当创建套接字成功的时候，该函数返回一个int类型的值，也就是socket描述符，该值大于等于0；而如果创建套接字失败时则返回-1。

bind()
------

函数原型：

.. code:: c

        int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);

在套接口中，一个套接字只是用户程序与内核交互信息的枢纽，它自身没有太多的信息，也没有网络协议地址和端口号等信息，在进行网络通信的时候，必须把一个套接字与一个IP地址或端口号相关联，这个过程就是绑定的过程。

bind()函数用于将一个 IP
地址或端口号与一个套接字进行绑定，许多时候内核会帮我们自动绑定一个IP地址与端口号，然而有时用户可能需要自己来完成这个绑定的过程，以满足实际应用的需要，最典型的情况是一个服务器进程需要绑定一个众所周知的地址和端口以等待客户来连接，作为服务器端，这一步绑定的操作是必要的，而作为客户端，则不是必要的，因为内核会帮我们自动选择合适的IP地址与端口号。

    ps：bind()函数并不是总是需要调用的，只有用户进程想与一个具体的地址或端口相关联的时候才需要调用这个函数。如果用户进程没有这个需要，那么程序可以依赖内核的自动的选址机制来完成自动地址选择。

参数：

-  sockfd：sockfd是由socket()函数返回的套接字描述符。
-  my_addr：my_addr是一个指向套接字地址结构的指针。
-  addrlen：addrlen指定了以addr所指向的地址结构体的字节长度。

若bind()函数绑定成功则返回0，若出错则为-1。

sockaddr 结构内容如下：

sockaddr结构：

.. code:: c

    struct sockaddr {
        sa_family_t     sa_family;
        char            sa_data[14];
    }

咋一看这个结构体，好像没啥信息要我们填写的，确实也是这样子，我们需要填写的IP地址与端口号等信息，都在sa_data连续的14字节信息里面，但这个结构对用户操作不友好，一般我们在使用的时候都会使用sockaddr_in结构，sockaddr_in和sockaddr是并列的结构（占用的空间是一样的），指向sockaddr_in的结构体的指针也可以指向sockadd的结构体，并代替它，而且sockaddr_in结构对用户将更加友好，在使用的时候进行类型转换就可以了。

sockaddr_in结构：

.. code:: c

    struct sockaddr_in {
        short int sin_family;               /* 协议族 */
        unsigned short int sin_port;        /* 端口号 */
        struct in_addr sin_addr;            /* IP地址 */
        unsigned char sin_zero[8];          /* sin_zero是为了让sockaddr与sockaddr_in两个数据结构保持大小相同而保留的空字节 */
    };

这个结构体的第一个字段是与sockaddr结构体是一致的，而剩下的字段就是sa_data连续的14字节信息里面的内容，只不过从新定义了成员变量而已，sin_port字段是我们需要填写的端口号信息，sin_addr字段是我们需要填写的IP地址信息，剩下sin_zero
区域的8字节保留未用。

举个简单的使用实例：

.. code:: c

        struct sockaddr_in server;

        bzero(&server, sizeof(server));

        // assign IP, PORT
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(6666);

        // binding newly created socket to given IP and verification
        bind(sockfd, (struct sockaddr*)&server, sizeof(server));

connect()
---------

函数原型：

.. code:: c

        int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

这个connect()函数用于客户端中，将sockfd与远端IP地址、端口号进行绑定，在TCP客户端中调用这个函数将发生握手过程（会发送一个TCP连接请求），并最终建立一个TCP连接，而对于UDP协议来说，调用这个函数只是在sockfd中记录远端IP地址与端口号，而不发送任何数据，参数信息与bind()函数是一样的。

函数调用成功则返回0，失败返回-1，错误原因存于errno中。

connect()函数是套接字连接操作，对于TCP协议来说，connect()函数操作成功之后代表对应的套接字已与远端主机建立了连接，可以发送与接收数据。

对于UDP协议来说，没有连接的概念，在这里我就将其描述为记录远端主机的IP地址与端口好，UDP协议经过connect()函数调用成功之后，在通过sendto()函数发送数据报时不需要指定目的地址、端口，因为此时已经记录到了远端主机的IP地址与端口号。UDP协议还可以给同一个套接字进行多次connect()操作，而TCP协议不可以，TCP只能指定一次connect操作。

listen()
--------

listen()函数只能在TCP服务器进程中使用，让服务器进程进入监听状态，等待客户端的连接请求，listen()函数在一般在bind()函数之后调用，在accept()函数之前调用，它的函数原型是：

.. code:: c

        int listen(int s, int backlog);

参数：

-  sockfd：sockfd是由socket()函数返回的套接字描述符。
-  backlog参数用来描述sockfd的等待连接队列能够达到的最大值。在服务器进程正处理客户端连接请求的时候，可能还存在其它的客户端请求建立连接，因为TCP连接是一个过程，由于同时尝试连接的用户过多，使得服务器进程无法快速地完成所有的连接请求，那怎么办呢？直接丢掉其他客户端的连接肯定不是一个很好的解决方法。因此内核会在自己的进程空间里维护一个队列，这些连接请求就会被放入一个队列中，服务器进程会按照先来后到的顺序去处理这些连接请求，这样的一个队列内核不可能让其任意大，所以必须有一个大小的上限，这个backlog告诉内核使用这个数值作为队列的上限。而当一个客户端的连接请求到达并且该队列为满时，客户端可能会收到一个表示连接失败的错误，本次请求会被丢弃不作处理。

accept()函数
------------

函数原型：

.. code:: c

        int accept(int s, struct sockaddr *addr, socklen_t *addrlen);

为了能够正常让TCP客户端能正常连接到服务器，服务器必须遵循以下流程处理：

1. 调用socket()函数创建对应的套接字类型。

2. 调用bind()函数将套接字绑定到本地的一个端口地址。

3. 调用listen()函数让服务器进程进入监听状态，等待客户端的连接请求。

4. 调用accept()函数处理到来的连接请求。

accept()函数用于TCP服务器中，等待着远端主机的连接请求，并且建立一个新的TCP连接，在调用这个函数之前需要通过调用listen()函数让服务器进入监听状态，如果队列中没有未完成连接套接字，并且套接字没有标记为非阻塞模式，accept()函数的调用会阻塞应用程序直至与远程主机建立TCP连接；如果一个套接字被标记为非阻塞式而队列中没有未完成连接套接字,
调用accept()函数将立即返回EAGAIN。

所以，accept()函数就是用于处理连接请求的，它会从未完成连接队列中取出第一个连接请求，建一个和参数
s 属性相同的连接套接字，并为这个套接字分配一个文件描述符,
然后以这个描述符返回，新创建的描述符不再处于监听状态，原套接字 s
不受此调用的影响，还是会处于监听状态，因为 s
是由socket()函数创建的，而处理连接时accept()函数会创建另一个套接字。

参数addr用来返回已连接的客户端的IP地址与端口号，参数addrlen用于返回addr所指向的地址结构体的字节长度，如果我们对客户端的IP地址与端口号不感兴趣，可以把arrd和addrlen均置为空指针。

若连接成功则返回一个socket描述符（非负值），若出错则为-1。

    ps:
    如果accept()连接成功，那么其返回值是由内核自动生成的一个全新描述符，代表与客户端的TCP连接，一个服务器通常仅仅创建一个监听套接字，它在该服务器生命周期内一直存在，内核为每个由服务器进程接受的客户端连接创建一个已连接套接字。

read()
------

一旦客户端与服务器建立好TCP连接之后，我们就可以通过sockfd套接字描述符来收发数据，这与我们读写文件是差不多的操作，接收网络中的数据函数可以是read()、recv()、recvfrom()等。

函数原型：

.. code:: c

        ssize_t read(int fd, void *buf, size_t count);

        ssize_t recv(int sockfd, void *buf, size_t len, int flags);

        ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen);

    ps：ssize_t 它表示的是 signed size_t 类型。

read() 从描述符 fd
（描述符可以是文件描述符也可以是套接字描述符，本章主要讲解套接字，此处为套接字描述符）中读取
count 字节的数据并放入从 buf
开始的缓冲区中，read()函数调用成功返回读取到的字节数，此返回值受文件剩余字节数限制，当返回值小于指定的字节数时
并不意味着错误；这可能是因为当前可读取的字节数小于指定的
字节数（比如已经接近文件结尾，或者正在从管道或者终端读取数据，或者read()函数被信号中断等），出错返回-1并设置errno，如果在调read之前已到达文件末尾，则这次read返回0。

参数： - fd：在socket编程中是指定套接字描述符。 -
buf：指定存放数据的地址。 -
count：是指定读取的字节数，将读取到的数据保存在缓冲区buf中。

错误代码： - EINTR：在读取到数据前被信号所中断。 - EAGAIN：使用
O_NONBLOCK 标志指定了非阻塞式输入输出，但当前没有数据可读。 -
EIO：输入输出错误，可能是正处于后台进程组进程试图读取其控制终端，但读操作无效，或者被信号SIGTTIN所阻塞,
或者其进程组是孤儿进程组，也可能执行的是读磁盘或者磁带机这样的底层输入输出错误。
- EISDIR：fd 指向一个目录。 - EBADF：fd
不是一个合法的套接字描述符，或者不是为读操作而打开。 - EINVAL：fd
所连接的对象不可读。 - EFAULT：buf 超出用户可访问的地址空间。

recv()
------

函数原型：

.. code:: c

        ssize_t recv(int sockfd, void *buf, size_t len, int flags);

不论是客户还是服务器应用程序都可以用recv()函数从TCP连接的另一端接收数据，它与read()函数的功能是差不多的。

recv()函数会先检查套接字 s 的接收缓冲区，如果 s
接收缓冲区中没有数据或者协议正在接收数据，那么recv就一直等待，直到协议把数据接收完毕。当协议把数据接收完毕，recv()函数就把
s 的接收缓冲中的数据拷贝到 buf
中，但是要注意的是议接收到的数据可能大于buf的长度，所以在这种情况下要调用几次recv()函数才能把s的接收缓冲中的数据拷贝完。recv()函数仅仅是拷贝数据，真正的接收数据是由协议来完成的，recv函数返回其实际拷贝的字节数。如果recv()函数在拷贝时出错，那么它返回SOCKET_ERROR；如果recv()函数在等待协议接收数据时网络中断了，那么它返回0。

参数：

-  sockfd：指定接收端套接字描述符。
-  buf：指定一个接收数据的缓冲区，该缓冲区用来存放recv()函数接收到的数据。
-  len：指定recv()函数拷贝的数据长度。

参数 flags 一般设置为0即可，其他数值定义如下: -
MSG_OOB：接收以out-of-band送出的数据。 -
MSG_PEEK：保持原有数据，就是说接收到的数据并不会被删除,
如果再调用recv()函数还会拷贝相同的数据到buf中。 -
MSG_WAITALL：强迫接收到指定len大小的数据后才能返回,
除非有错误或信号产生。 -
MSG_NOSIGNAL：recv()函数不会被SIGPIPE信号中断，返回值成功则返回接收到的字符数,
失败返回-1，错误原因存于errno中。

错误代码：

-  EBADF：fd 不是一个合法的套接字描述符，或者不是为读操作而打开。
-  EFAULT：buf 超出用户可访问的地址空间。
-  ENOTSOCK：参数 s 为一文件描述词, 非socket.
-  EINTR：在读取到数据前被信号所中断。
-  EAGAIN：此动作会令进程阻塞, 但参数s的 socket 为不可阻塞。
-  ENOBUFS：buf内存空间不足。
-  ENOMEM：内存不足。
-  EINVAL：传入的参数不正确。

write()
-------

函数原型：

.. code:: c

        ssize_t write(int fd, const void *buf, size_t count);

write()函数一般用于处于稳定的TCP连接中传输数据，当然也能用于UDP协议中，它向套接字描述符
fd 中写入 count 字节的数据，数据起始地址由 buf
指定，函数调用成功返回写的字节数，失败返回-1，并设置errno变量。

在网络编程中，当我们向套接字描述符写数据时有两种可能：

1. write()函数的返回值大于0，表示写了部分数据或者是全部的数据，这样我们可以使用一个while循环不断的写入数据，但是循环过程中的
   buf 参数和 count
   参数是我们自己来更新的，也就是说，网络编程中写函数是不负责将全部数据写完之后再返回的，说不定中途就返回了！

2. 返回值小于0，此时出错了，需要根据错误类型进行相应的处理。

所以一般我们处理写数据的时候都会自己封装一层，以保证数据的正确写入：

.. code:: c

    /* Write "n" bytes to a descriptor. */
    ssize_t writen(int fd, const void *vptr, size_t n)
    {
        size_t      nleft;      //剩余要写的字节数
        ssize_t     nwritten;   //已经写的字节数
        const char  *ptr;       //write的缓冲区

        ptr = vptr;             //把传参进来的write要写的缓冲区备份一份
        nleft = n;              //还剩余需要写的字节数初始化为总共需要写的字节数

        //检查传参进来的需要写的字节数的有效性
        while (nleft > 0) {     
            if ( (nwritten = write(fd, ptr, nleft)) <= 0) { //把ptr写入fd
                if (nwritten < 0 && errno == EINTR) //当write返回值小于0且因为是被信号打断
                    nwritten = 0;       /* and call write() again */
                else
                    return(-1);         /* error 其他小于0的情况为错误*/
            }

            nleft -= nwritten;          //还剩余需要写的字节数=现在还剩余需要写的字节数-这次已经写的字节数
            ptr += nwritten;          //下次开始写的缓冲区位置=缓冲区现在的位置右移已经写了的字节数大小
        }
        return(n); //返回已经写了的字节数
    }

    ps：当然啦，如果是比较简单的数据（比如单行数据）倒是不需要那么麻烦，直接调用write()也是完全没有问题的，只是看情况写代码就行了，上面代码的封装只是保证程序的健壮性。

    注意，这个函数在写入数据完成后并不是立即发送的，至于什么时候发送则由TCP/IP协议栈决定。

send()
------

函数原型：

.. code:: c

        int send(int s, const void *msg, size_t len, int flags);

无论是客户端还是服务器应用程序都可以用send()函数来向TCP连接的另一端发送数据。

参数：

-  s：指定发送端套接字描述符。
-  msg：指定要发送数据的缓冲区。
-  len：指明实际要发送的数据的字节数。
-  flags：一般设置为0即可

当调用该函数时，send()函数先比较待发送数据的长度len和套接字s的发送缓冲的长度，如果len大于s的发送缓冲区的长度，该函数返回SOCKET_ERROR；如果len小于或者等于s的发送缓冲区的长度，那么send()函数先检查协议是否正在发送s的发送缓冲中的数据，如果是就等待协议把数据发送完，如果协议还没有开始发送s的发送缓冲中的数据或者s的发送缓冲中没有数据，那么send()函数就比较s的发送缓冲区的剩余空间和len，如果len大于剩余空间大小send()函数就一直等待协议把s的发送缓冲中的数据发送完，如果len小于剩余空间大小send()函数就仅仅把buf中的数据拷贝到s的发送缓冲区的剩余空间里。

如果send()函数拷贝数据成功，就返回实际copy的字节数，如果send()函数在拷贝数据时出现错误，那么send就返回SOCKET_ERROR；如果send在等待协议传送数据时网络断开的话，那么send函数也返回SOCKET_ERROR。

    注意send()函数把buf中的数据成功拷贝到s的发送缓冲的剩余空间里后它就返回了，但是此时这些数据并不一定马上被传到连接的另一端。

sendto
------

函数原型：

.. code:: c

    int sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);

sendto()函数与send函数非常像，但是它会通过 struct sockaddr 指向的 to
结构体指定要发送给哪个远端主机，在to参数中需要指定远端主机的IP地址、端口号等，而tolen参数则是指定to
结构体的字节长度。

close()
-------

函数原型：

.. code:: c

    int close(int fd);

close()函数是用于关闭一个指定的套接字，在关闭套接字后，将无法使用对应的套接字描述符，这个函数比较简单，当你不需要使用某个套接字描述符时，就将其关闭即可，在UDP协议中，close会释放一个套接字描述符的资源；而在TCP协议中，当调用close()函数后将发起“四次挥手”终止连接，当连接正式终止后，套接字描述符的资源才会被释放。

ioctlsocket()
-------------

函数原型：

.. code:: c

        int ioctlsocket( int s, long cmd, u_long *argp);

该函数用于获取与设置套接字相关的操作参数。

参数：

1. s：指定要操作的套接字描述符。
2. cmd：对套接字s的操作命令。

-  FIONBIO：命令用于允许或禁止套接字的非阻塞模式。在这个命令下，argp参数指向一个无符号长整型，如果该值为0则表示禁止非阻塞模式，而如果该值非0则表示允许非阻塞模式则。当创建一个套接字的时候，它就处于阻塞模式，也就是说非阻塞模式被禁止，这种情况下所有的发送、接收函数都会是阻塞的，直至发送、接收成功才得以继续运行；而如果是非阻塞模式下，所有的发送、接收函数都是不阻塞的，如果发送不出去或者接收不到数据，将直接返回错误代码给用户，这就需要用户对这些“意外”情况进行处理，保证代码的健壮性。
-  FIONREAD：FIONREAD命令确定套接字s自动读入的数据量，这些数据已经被接收，但应用线程并未读取的，所以可以使用这个函数来获取这些数据的长度，在这个命令状态下，argp参数指向一个无符号长整型，用于保存函数的返回值（即未读数据的长度）。如果套接字是SOCK_STREAM类型，则FIONREAD命令会返回recv()函数中所接收的所有数据量，这通常与在套接字接收缓存队列中排队的数据总量相同；而如果套接字是SOCK_DGRAM类型的，则FIONREAD命令将返回在套接字接收缓存队列中排队的第一个数据包大小。
-  SIOCATMARK：确认是否所有的带外数据都已被读入。

3. argp：指向cmd命令所带参数的指针。

其实这个函数，举个例子：

.. code:: c

    // 控制为阻塞模式。
    u_long mode = 0;
    ioctlsocket(s,FIONBIO,&mode);

    // 控制为非阻塞模式。
    u_long mode = 1;
    ioctlsocket(s,FIONBIO,&mode); 

getsockopt()、setsockopt()
--------------------------

.. code:: c

        int getsockopt(int sockfd, int level, int optname,
                        void *optval, socklen_t *optlen);

        int setsockopt(int sockfd, int level, int optname,
                        const void *optval, socklen_t optlen);

看名字就知道，这个函数是用于获取/设置套接字的一些选项的，参数level有多个常见的选项，如：

-  SOL_SOCKET：表示在Socket层。
-  IPPROTO_TCP：表示在TCP层。
-  IPPROTO_IP： 表示在IP层。

参数optname表示该层的具体选项名称，比如：

-  对于SOL_SOCKET选项，可以是SO_REUSEADDR（允许重用本地地址和端口）、SO_SNDTIMEO（设置发送数据超时时间）、SO_SNDTIMEO（设置接收数据超时时间）、SO_RCVBUF（设置发送数据缓冲区大小）等等。
-  对于IPPROTO_TCP选项，可以是TCP_NODELAY（不使用Nagle算法）、TCP_KEEPALIVE（设置TCP保活时间）等等。
-  对于IPPROTO_IP选项，可以是IP_TTL（设置生存时间）、IP_TOS（设置服务类型）等等。

TCP客户端实验
-------------

我们本小节就通过socket
API函数去实现一个TCP客户端，代码的步骤首先是与服务器建立连接，然后在客户端中输入一些数据并且将它发送到服务器，最后在数据发送完毕后就终止连接，由于TCP协议的模型是
``客户端 <-> 服务器`` ，因此我们在下一小节还会实现一个TCP服务器，两个进程间相互通信。

首先明确一下整个客户端的流程步骤： 1.
调用socket()函数创建一个套接字描述符。 2.
调用connect()函数连接到指定服务器中，端口号为服务器监听的端口号。 3.
调用write()函数发送数据。 4. 调用close()函数终止连接。

TCP客户端代码：

.. code:: c

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>

    #define HOST "192.168.0.217"        // 根据你服务器的IP地址修改
    #define PORT 6666                   // 根据你服务器进程绑定的端口号修改
    #define BUFFER_SIZ (4 * 1024)           // 4k的数据区域


    int main(void)
    {
        int sockfd, ret;
        struct sockaddr_in server;
        char buffer[BUFFER_SIZ];        //用于保存输入的文本

        // 创建套接字描述符
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("create an endpoint for communication fail!\n");
            exit(1);
        } 

        bzero(&server, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.s_addr = inet_addr(HOST);

        // 建立TCP连接
        if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
            printf("connect server fail...\n");
            close(sockfd);
            exit(1);
        } 

        printf("connect server success...\n");

        while (1) {

            printf("please enter some text: ");
            fgets(buffer, BUFFER_SIZ, stdin);

            //输入了end，退出循环（程序）
            if(strncmp(buffer, "end", 3) == 0)
                break;

            write(sockfd, buffer, sizeof(buffer));
        }

        close(sockfd);
        exit(0);
    }

TCP服务器实验
-------------

接着我们实现一个服务器代码，接受客户端的连接，并且将来自客户端的数据打印到终端中。

服务器的代码流程如下：

1. 调用socket()函数创建一个套接字描述符。
2. 调用bind()函数绑定监听的端口号。
3. 调用listen()函数让服务器进入监听状态。
4. 调用accept()函数处理来自客户端的连接请求。
5. 调用read()函数接收客户端发送的数据。
6. 调用close()函数终止连接。

服务器代码：

.. code:: c

    #include <stdio.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/socket.h>
    #include <sys/types.h>

    #define MAX 10*1024
    #define PORT 6666

    // Driver function
    int main()
    {
        char buff[MAX];
        int n;
        int sockfd, connfd, len;
        struct sockaddr_in server, client;

        // socket create and verification
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("socket creation failed...\n");
            exit(0);
        }

        printf("socket successfully created..\n");
        bzero(&server, sizeof(server));

        // assign IP, PORT
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(PORT);

        // binding newly created socket to given IP and verification
        if ((bind(sockfd, (struct sockaddr*)&server, sizeof(server))) != 0) {
            printf("socket bind failed...\n");
            exit(0);
        }
        
        printf("socket successfully binded..\n");

        // now server is ready to listen and verification
        if ((listen(sockfd, 5)) != 0) {
            printf("Listen failed...\n");
            exit(0);
        }

        printf("server listening...\n");
        
        len = sizeof(client);

        // accept the data packet from client and verification
        connfd = accept(sockfd, (struct sockaddr*)&client, &len);
        if (connfd < 0) {
            printf("server acccept failed...\n");
            exit(0);
        }

        printf("server acccept the client...\n");

        // infinite loop for chat
        while(1) {
            bzero(buff, MAX);

            // read the messtruct sockaddrge from client and copy it in buffer
            if (read(connfd, buff, sizeof(buff)) <= 0) {
                printf("client close...\n");
                close(connfd);
                break;
            }

            // print buffer which contains the client contents
            printf("from client: %s\n", buff);

            // if msg contains "Exit" then server exit and chat ended.
            if (strncmp("exit", buff, 4) == 0) {
                printf("server exit...\n");
                close(connfd);
                break;
            }
        }

        // After chatting close the socket
        close(sockfd);
        exit(0);
    }

实验现象
--------

分别进入 ``embed_linux_tutorial/base_code/system_programing/tcp_client`` 与 ``embed_linux_tutorial/base_code/system_programing/tcp_server`` 目录下运行make命令将客户端与服务器的代码编译，打开两个终端，然后首先运行服务器的程序，接着运行客户端的程序，然后在客户端程序中输入想要发送的数据，然后发送出去，你就会看到在服务器进程中将数据打印出来，现象如下：

客户端进程：

.. code:: c

    ➜  tcp_client git:(master)  ./targets 

    connect server success...
    please enter some text: abcdefg
    please enter some text: aaaaaaaaaaaa
    please enter some text: bbbbbbbbbbbbbbbbbbb
    please enter some text: 6666666666666666666666666
    please enter some text: embedfire socket api      
    please enter some text: 野火
    please enter some text: exit
    ➜  tcp_client git:(master) 

服务器进程：

.. code:: c

    ➜  tcp_server git:(master) ./targets     

    socket successfully created..
    socket successfully binded..
    server listening...
    server acccept the client...
    from client: abcdefg

    from client: aaaaaaaaaaaa

    from client: bbbbbbbbbbbbbbbbbbb

    from client: 6666666666666666666666666

    from client: embedfire socket api

    from client: 野火

    client close...
    ➜  tcp_server git:(master)

