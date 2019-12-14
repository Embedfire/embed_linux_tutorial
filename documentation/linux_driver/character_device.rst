.. vim: syntax=rst

字符设备驱动
------

本章节内容处于开发笔记状态，还待整理至最终版的教程。

本章节内容处于开发笔记状态，还待整理至最终版的教程。

本章，我们将学习如何编写一个字符设备驱动：

1. 介绍Linux设备分类，了解什么是字符设备、块设备以及网络设备；

2. 我们经常说“一切皆文件”，对于硬件设备，也是如此。那当我们打开设备文件时，到底做了什么东西。为什么我们对该文件进行读写可以操作我们的硬件呢。刚开始学习本节，可能会很懵圈。但是，当你学习完整章的内容之后，再回过头来看本节的内容，可以加深你对字符设备驱动的理解。

3. 介绍字符设备驱动相关的数据结构

4. 介绍字符设备驱动程序的基本框架，以后写字符设备驱动时，就可以根据这个模板进行填空了。

5. 自己写一个字符设备驱动

Linux设备
~~~~~~~

Linux中，根据设备的类型可以分为三类：字符设备、块设备和网络设备。

字符设备：应用程序按字节/字符来读写数据，通常不支持随机存取。我们常用的键盘、串口都是字符设备。

块设备：应用程序可以随机访问设备数据。典型的块设备有硬盘、SD卡、闪存等，应用程序可以寻址磁盘上的任何位置，并由此读取数据。此外，数据的读写只能以块的倍数进行。

网络设备是一种特殊设备，它并不存在于/dev下面，主要用于网络数据的收发。

open函数到底做了什么
~~~~~~~~~~~~

在学习字符设备驱动之前，我们一起了解一下，使用open函数打开设备文件，到底做了些什么工作？图 57‑1中列出了open函数执行的大致过程。

|charac002|

图 57‑1执行流程

设备文件通常在开机启动时自动创建的，不过，我们仍然可以使用命令mknod来创建一个新的设备文件，命令的基本语法如下：

mknod 设备名 设备类型 主设备号 次设备号

当我们使用上述命令，创建了一个字符设备文件时，实际上就是创建了一个设备节点inode结构体，并且将该设备的设备编号记录在成员i_rdev，将成员f_op指针指向了def_chr_fops结构体。这就是mknod负责的工作内容，具体代码见代码清单 57‑1。

代码清单 57‑1mknod调用关系

1 static struct inode \*shmem_get_inode(struct super_block \*sb, const struct inode \*dir,

2 umode_t mode, dev_t dev, unsigned long flags)

3 {

4 inode = new_inode(sb);

5 if (inode) {

6 ......

7 switch (mode & S_IFMT) {

8 default:

9 inode->i_op = &shmem_special_inode_operations;

10 init_special_inode(inode, mode, dev);

11 break;

12 ......

13 }

14 } else

15 shmem_free_inode(sb);

16 return inode;

17 }

18 void init_special_inode(struct inode \*inode, umode_t mode, dev_t rdev)

19 {

20 inode->i_mode = mode;

21 if (S_ISCHR(mode)) {

22 inode->i_fop = &def_chr_fops;

23 inode->i_rdev = rdev;

24 }

25 ....

26 }

命令mknod最终会调用init_special_inode函数，由于我们创建的是字符设备，因此，会执行第22~23行的代码。这样就完成了图 57‑1处的内容。

我们使用的open函数在内核中对应的是sys_open函数，sys_open函数又会调用do_sys_open函数。在do_sys_open函数中，首先调用函数get_unused_fd_flags来获取一个未被使用的文件描述符fd，该文件描述符就是我们最终通过open函数得到的值。紧接着，又调用了
do_filp_open函数，该函数通过调用函数get_empty_filp得到一个新的file结构体，之后的代码做了许多复杂的工作，如解析文件路径，查找该文件的文件节点inode等，直接来到了函数do_dentry_open函数，见代码清单 57‑2。

代码清单 57‑2 do_dentry_open函数（位于内核源码/fs/open.c文件）

1 static int do_dentry_open(struct file \*f,

2 struct inode \*inode,

3 int (*open)(struct inode \*, struct file \*),

4 const struct cred \*cred)

5 {

6 ……

7 f->f_op = fops_get(inode->i_fop);

8 ……

9

10 if (!open)

11 open = f->f_op->open;

12 if (open) {

13 error = open(inode, f);

14 if (error)

15 goto cleanup_all;

16 }

17 ……

18 }

代码清单 57‑2中的第7行使用fops_get函数来获取该文件节点inode的成员变量i_fop，还记得图 57‑1中的处吗？我们使用mknod创建字符设备文件时，将def_chr_fops结构体赋值给了该设备文件inode的i_fop成员。到了这里，我们新建的file结构体的成员f_op就指向了
def_chr_fops。

代码清单 57‑3 def_chr_fops结构体（位于内核源码/fs/char_dev.c文件）

1 const struct file_operations def_chr_fops = {

2 .open = chrdev_open,

3 .llseek = noop_llseek,

4 };

最终，会执行def_chr_fops中的open函数，也就是chrdev_open函数，可以理解为一个字符设备的通用初始化函数，根据字符设备的设备号，找到相应的字符设备，从而得到操作该设备的方法，代码实现见代码清单 57‑5。

|charac003|

代码清单 57‑4 图解chrdev_open函数

代码清单 57‑5 chrdev_open函数（位于内核源码/fs/char_dev.c文件）

1 static int chrdev_open(struct inode \*inode, struct file \*filp)

2 {

3 const struct file_operations \*fops;

4 struct cdev \*p;

5 struct cdev \*new = NULL;

6 int ret = 0;

7

8 spin_lock(&cdev_lock);

9 p = inode->i_cdev;

10 if (!p) {

11 struct kobject \*kobj;

12 int idx;

13 spin_unlock(&cdev_lock);

14 kobj = kobj_lookup(cdev_map, inode->i_rdev, &idx);

15 if (!kobj)

16 return -ENXIO;

17 new = container_of(kobj, struct cdev, kobj);

18 spin_lock(&cdev_lock);

19 /\* Check i_cdev again in case somebody beat us to it while

20 we dropped the lock.
\*/

21 p = inode->i_cdev;

22 if (!p) {

23 inode->i_cdev = p = new;

24 list_add(&inode->i_devices, &p->list);

25 new = NULL;

26 } else if (!cdev_get(p))

27 ret = -ENXIO;

28 } else if (!cdev_get(p))

29 ret = -ENXIO;

30 spin_unlock(&cdev_lock);

31 cdev_put(new);

32 if (ret)

33 return ret;

34

35 ret = -ENXIO;

36 fops = fops_get(p->ops);

37 if (!fops)

38 goto out_cdev_put;

39

40 replace_fops(filp, fops);

41 if (filp->f_op->open) {

42 ret = filp->f_op->open(inode, filp);

43 if (ret)

44 goto out_cdev_put;

45 }

46

47 return 0;

48

49 out_cdev_put:

50 cdev_put(p);

51 return ret;

52 }

在Linux内核中，使用结构体cdev来描述一个字符设备。代码清单 57‑5中的第14行，inode->i_rdev中保存了字符设备的设备编号，通过函数kobj_lookup函数便可以找到该设备文件cdev结构体的kobj成员，再通过函数container_of便可以得到该字符设备对应的结构体cde
v。函数container_of的作用就是通过一个结构变量中一个成员的地址找到这个结构体变量的首地址。同时，将cdev结构体记录到文件节点inode中的i_cdev，便于下次打开该文件。继续阅读第36~45行代码，我们可以发现，函数chrdev_open最终将该文件结构体file的成员f_op替换成
了cdev对应的ops成员，并执行ops结构体中的open函数。

最后，调用图 57‑1的fd_install函数，完成文件描述符和文件结构体file的关联，之后我们使用对该文件描述符fd调用read、write函数，最终都会调用file结构体对应的函数，实际上也就是调用cdev结构体中ops结构体内的相关函数。

总结一下整个过程，当我们使用open函数，打开设备文件时，会根据该设备的文件的设备号找到相应的设备结构体，从而得到了操作该设备的方法。也就是说如果我们要添加一个新设备的话，我们需要提供一个设备号，一个设备结构体以及操作该设备的方法（file_operations结构体）
。接下来，我们将介绍以上的三个内容。

数据结构
~~~~

本节，我们讲解编写设备驱动需要了解到的数据结构体，包括了文件操作方式（file_operations），字符设备结构体（struct cdev）以及文件描述结构体（struct file）。

file_operations结构体
^^^^^^^^^^^^^^^^^^

上一节，我们提及到的文件结构体file以及字符设备结构体cdev，他们都有一个struct file_operations类型的成员变量。file_operations结构体中包含了操作文件的一系列函数指针，代码清单 57‑6中只列出本章使用到的部分函数。

代码清单 57‑6 file_operations结构体（位于内核源码/include/linux/fs.h文件）

1 struct file_operations {

2

3 loff_t (*llseek) (struct file \*, loff_t, int);

4 ssize_t (*read) (struct file \*, char \__user \*, size_t, loff_t \*);

5 ssize_t (*write) (struct file \*, const char \__user \*, size_t, loff_t \*);

6 long (*unlocked_ioctl) (struct file \*, unsigned int, unsigned long);

7 int (*open) (struct inode \*, struct file \*)

8 int (*release) (struct inode \*, struct file \*);

9 };

-  llseek：用于修改文件的当前读写位置，并返回偏移后的位置。参数file传入了对应的文件指针，我们可以看到代码清单
  57‑6中所有的函数都有该形参，通常用于读取文件的信息，如文件类型、读写权限；参数loff_t指定偏移量的大小；参数int是用于指定新位置指定成从文件的某个位置进行偏移，SEEK_SET表示从文件起始处开始偏移；SEEK_CUR表示从当前位置开始偏移；SEEK_END表示从文件结尾开始偏移。

-  read：用于读取设备中的数据，并返回成功读取的字节数。该函数指针被设置为NULL时，会导致系统调用read函数报错，提示“非法参数”。该函数有三个参数：file类型指针变量，char
  \__user*类型的数据缓冲区，__user用于修饰变量，表明该变量所在的地址空间是用户空间的。内核模块不能直接使用该数据，需要使用copy_to_user函数来进行操作。size_t类型变量指定读取的数据大小。

-  write：用于向设备写入数据，并返回成功写入的字节数，write函数的参数用法与read函数类似，不过在访问__user修饰的数据缓冲区，需要使用copy_from_user函数。

-  unlocked_ioctl：提供设备执行相关控制命令的实现方法，它对应于应用程序的fcntl函数以及ioctl函数。在 kernel 3.0 中已经完全删除了 struct file_operations 中的 ioctl 函数指针。

-  open：设备驱动第一个被执行的函数，一般用于硬件的初始化。如果该成员被设置为NULL，则表示这个设备的打开操作永远成功。

-  release：当file结构体被释放时，将会调用该函数。与open函数相反，该函数可以用于释放

上面，我们提到read和write函数时，需要使用copy_to_user函数以及copy_from_user函数来进行数据访问，写入/读取成功函数返回0，失败则会返回未被拷贝的字节数。

代码清单 57‑7copy_to_user和copy_from_user函数（位于内核源码/include/asm-generic/uaccess.h文件）

1 static inline long copy_from_user(void \*to,

2 const void \__user \* from, unsigned long n)

3 static inline long copy_to_user(void \__user \*to,

4 const void \*from, unsigned long n)

-  to：指定目标地址，也就是数据存放的地址，

-  from：指定源地址，也就是数据的来源。

-  n：指定写入/读取数据的字节数。

file结构体
^^^^^^^

内核中用file结构体来表示每个打开的文件，每打开一个文件，内核会创建一个结构体，并将对该文件上的操作函数传递给该结构体的成员变量f_op。代码清单 57‑8中，只列出了我们本章需要了解的成员变量。

代码清单 57‑8 file结构体（位于内核源码/include/fs.h文件）

1 struct file {

2 const struct file_operations \*f_op;

3 /\* needed for tty driver, and maybe others \*/

4 void \*private_data;

5 };

-  f_op：存放与文件操作相关的一系列函数指针，如open、read、wirte等函数。

-  private_data：该指针变量只会用于设备驱动程序中，内核并不会对该成员进行操作。因此，在驱动程序中，通常用于指向描述设备的结构体。

cdev结构体
^^^^^^^

如图 57‑2所示，内核用struct cdev结构体来描述一个字符设备，并通过struct kobj_map类型的散列表cdev_map来管理当前系统中的所有字符设备。

|charac004|

图 57‑2 cdev_map与cdev的关系

代码清单 57‑9 cdev结构体（位于内核源码/include/linux/cdev.h文件）

1 struct cdev {

2 struct kobject kobj;

3 struct module \*owner;

4 const struct file_operations \*ops;

5 struct list_head list;

6 dev_t dev;

7 unsigned int count;

8 };

-  kobj：内核数据对象，用于管理该结构体。代码清单 57‑5中通过kobj_lookup函数中从cdev_map中得到该成员，由该成员便可以得到相应的字符设备结构体。

-  owner：指向了关联该设备的内核模块，实际上就是关联了驱动程序，通常设置为THIS_MODULE。

-  ops：该结构体中最重要的一部分，也是我们实现字符设备驱动的关键一步，用于存放所有操作该设备的函数指针。

-  list：实现一个链表，用于包含与该结构体对应的字符设备文件inode的成员i_devices 的链表。

-  dev：记录了字符设备的设备号。

-  count：记录了与该字符设备使用的次设备号的个数。

字符设备驱动程序框架
~~~~~~~~~~

前面我们已经讲解了一些相关的数据结构，但是各个结构体要如何进行联系？答案肯定是通过函数。因此，本节我们开始讲解关于字符设备的驱动程序框架。关于框架，我们在内核模块那张也讲了一个内核模块的框架，实际上，在Linux上写驱动程序，都是做一些“填空题”。因为Linux给我们提供了一个基本的框架，如果你不按
照这个框架写驱动，那么你写的驱动程序是不能被内核所接纳的。

初始化/移除字符设备
^^^^^^^^^^

Linux内核提供了两种方式来定义字符设备，见代码清单 57‑10。

代码清单 57‑10 定义字符设备

1 //第一种方式

2 static struct cdev chrdev;

3 //第二种方式

4 struct cdev \*cdev_alloc(void);

第一种方式，就是我们常见的变量定义；第二种方式，是内核提供的动态分配方式，调用该函数之后，会返回一个struct cdev类型的指针，用于描述字符设备。

从内核中移除某个字符设备，则需要调用cdev_del函数，见代码清单 57‑11。

代码清单 57‑11 cdev_del函数

1 void cdev_del(struct cdev \*p)

该函数需要将我们的字符设备结构体的地址作为实参传递进去，就可以从内核中移除该字符设备了。

分配/注销设备号
^^^^^^^^

Linux的各种设备都以文件的形式存放在/dev目录下，为了管理这些设备，系统为各个设备进行编号，每个设备号又分为主设备号和次设备号。主设备号用来区分不同种类的设备，如USB，tty等，次设备号用来区分同一类型的多个设备，如tty0，tty1……图
57‑3列出了部分tty设备，他们的主设备号都是4，而不同的次设备号分别对应一个tty设备。

|charac005|

图 57‑3 tty设备

内核提供了一种数据类型：dev_t，用于记录设备编号，该数据类型实际上是一个无符号32位整型，其中的12位用于表示主设备号，剩余的20位则用于表示次设备号。

实际上，内核将一部分主设备号分配给了一些常见的设备。在内核源码的Documentation/devices.txt文件中可以找到这些设备以及这部分设备占据的主设备号。

|charac006|

图 57‑4 devices文件

devices文件大致上分成了图 57‑4的四个部分：

1. 这一部分的内容，主要记录了当前内核所占据的所有字符设备的主设备号，我们通过检查这一列的内容，便可以知道当前的主设备号是否被内核占用。

2. 第二部分的内容，主要记录了设备的类型，主要分为块设备（block）以及字符设备（char），我们这里只关心字符设备即可。

3. 第三部分的内容，记录了每个次设备号对应的设备。

4. 第四部分的内容，则是对每个设备的概述。

根据上一节提到的，创建一个新的字符设备之前，我们需要为新的字符设备注册一个新的设备号，就好像每个人都有一个身份证号，用来标识自己。内核提供了三种方式，来完成这项工作。

register_chrdev_region函数
''''''''''''''''''''''''

register_chrdev_region函数用于静态地为一个字符设备申请一个或多个设备编号。该函数在分配成功时，会返回0；失败则会返回相应的错误码，函数原型见代码清单 57‑12。

代码清单 57‑12 register_chrdev_region函数原型

1 int register_chrdev_region(dev_t from, unsigned count, const char \*name)

参数说明：

-  from：dev_t类型的变量，用于指定字符设备的起始设备号，如果要注册的设备号已经被其他的设备注册了，那么就会导致注册失败。

-  count：指定要申请的设备号个数，count的值不可以太大，否则会与下一个主设备号重叠。

-  name：用于指定该设备的名称，我们可以在/proc/devices中看到该设备。

register_chrdev_region函数使用时需要指定一个设备编号， Linux内核为我们提供了生成设备号的宏定义MKDEV，用于将主设备号和次设备号合成一个设备号，主设备可以通过查阅内核源码的Documentation/devices.txt文件，而次设备号通常是从编号0开始。除此之外，内
核还提供了另外两个宏定义MAJOR和MINOR，可以根据设备的设备号来获取设备的主设备号和次设备号。

代码清单 57‑13 合成设备号MKDEV（位于内核源码/include/linux/kdev_t.h）

1 #define MINORBITS 20

2 #define MINORMASK ((1U << MINORBITS) - 1)

3

4 #define MAJOR(dev) ((unsigned int) ((dev) >> MINORBITS))

5 #define MINOR(dev) ((unsigned int) ((dev) & MINORMASK))

6 #define MKDEV(ma,mi) (((ma) << MINORBITS) \| (mi))

alloc_chrdev_region函数
'''''''''''''''''''''

使用register_chrdev_region函数时，都需要去查阅内核源码的Documentation/devices.txt文件，这就十分不方便。因此，内核又为我们提供了一种能够动态分配设备编号的方式：alloc_chrdev_region。

调用alloc_chrdev_region函数，内核会自动分配给我们一个尚未使用的主设备号。我们可以通过命令“cat /proc/devices”查询内核分配的主设备号。

代码清单 57‑14 alloc_chrdev_region函数原型

1 int alloc_chrdev_region(dev_t \*dev, unsigned baseminor, unsigned count, const char \*name)

参数说明如下：

-  dev：指向dev_t类型数据的指针变量，用于存放分配到的设备编号的起始值；

-  baseminor：次设备号的起始值，通常情况下，设置为0；

-  count、name：同register_chrdev_region类型，用于指定需要分配的设备编号的个数以及设备的名称。

unregister_chrdev_region函数
''''''''''''''''''''''''''

当我们删除字符设备时候，我们需要把分配的设备编号交还给内核，对于使用register_chrdev_region函数以及alloc_chrdev_region函数分配得到的设备编号，可以使用unregister_chrdev_region函数实现该功能。

代码清单 57‑15 unregister_chrdev_region函数（位于内核源码/fs/char_dev.c）

1 void unregister_chrdev_region(dev_t from, unsigned count)

-  from：指定需要注销的字符设备的设备编号起始值，我们一般将定义的dev_t变量作为实参。

-  count：指定需要注销的字符设备编号的个数，该值应与申请函数的count值相等，通常采用宏定义进行管理。

register_chrdev函数
'''''''''''''''''

除了上述的两种，内核还提供了register_chrdev函数用于分配设备号。该函数是一个内联函数，它不仅支持静态申请设备号，也支持动态申请设备号，并将主设备号返回，函数原型见代码清单 57‑16。

代码清单 57‑16 register_chrdev函数原型（位于内核源码/include/linux/fs.h文件）

1 static inline int register_chrdev(unsigned int major, const char \*name,

2 const struct file_operations \*fops)

3 {

4 return \__register_chrdev(major, 0, 256, name, fops);

5 }

参数说明：

-  major：用于指定要申请的字符设备的主设备号，等价于register_chrdev_region函数，当设置为0时，内核会自动分配一个未使用的主设备号。

-  name：用于指定字符设备的名称

-  fops：用于操作该设备的函数接口指针。

我们从代码清单 57‑16中可以看到，使用register_chrdev函数向内核申请设备号，同一类字符设备（即主设备号相同），会在内核中申请了256个，通常情况下，我们不需要用到这么多个设备，这就造成了极大的资源浪费。

unregister_chrdev函数
'''''''''''''''''''

使用register函数申请的设备号，则应该使用unregister_chrdev函数进行注销。

代码清单 57‑17 unregister_chrdev函数（位于内核源码/include/linux/fs.h文件）

1 static inline void unregister_chrdev(unsigned int major, const char \*name)

2 {

3 \__unregister_chrdev(major, 0, 256, name);

4 }

-  major：指定需要释放的字符设备的主设备号，一般使用register_chrdev函数的返回值作为实参。

-  name：执行需要释放的字符设备的名称。

关联设备的操作方式
^^^^^^^^^

前面我们已经提到过了，编写一个字符设备最重要的事情，就是要实现file_operations这个结构体中的函数。实现之后，如何将该结构体与我们的字符设备结构相关联呢？内核提供了cdev_init函数，来实现这个工程。

代码清单 57‑18 cdev_init函数（位于内核源码/fs/char_dev.c）

1 void cdev_init(struct cdev \*cdev, const struct file_operations \*fops)

-  cdev：struct cdev类型的指针变量，指向需要关联的字符设备结构体；

-  fops：file_operations类型的结构体指针变量，一般将实现操作该设备的结构体file_operations结构体作为实参。

注册设备
^^^^

cdev_add函数用于向内核的cdev_map散列表添加一个新的字符设备，见代码清单 57‑19。

代码清单 57‑19 cdev_add函数（位于内核源码/fs/char_dev.c文件）

1 int cdev_add(struct cdev \*p, dev_t dev, unsigned count)

-  p：struct cdev类型的指针，用于指定需要添加的字符设备；

-  dev：dev_t类型变量，用于指定设备的起始编号；

-  count：指定注册多少个设备。

字符设备驱动程序实验
~~~~~~~~~~

结合前面所有的知识点，首先，字符设备驱动程序是以内核模块的形式存在的，因此，使用内核模块的程序框架是毫无疑问的。紧接着，我们要向系统注册一个新的字符设备，需要这几样东西：字符设备结构体cdev，设备编号devno，以及最最最重要的操作方式结构体file_operations。

下面，我们开始编写我们自己的字符设备驱动程序。

内核模块框架
^^^^^^

既然我们的设备程序是以内核模块的方式存在的，那么就需要先写出一个基本的内核框架，见代码清单 57‑20。

代码清单 57‑20 内核模块加载函数（位于文件chrdev.c）

1 #define DEV_NAME "EmbedCharDev"

2 #define DEV_CNT (1)

3 #define BUFF_SIZE 128

4 //定义字符设备的设备号

5 static dev_t devno;

6 //定义字符设备结构体chr_dev

7 static struct cdev chr_dev;

8 static int \__init chrdev_init(void)

9 {

10 int ret = 0;

11 printk("chrdev init\n");

12 //第一步

13 //采用动态分配的方式，获取设备编号，次设备号为0，

14 //设备名称为EmbedCharDev，可通过命令cat /proc/devices查看

15 //DEV_CNT为1，当前只申请一个设备编号

16 ret = alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);

17 if (ret < 0) {

18 printk("fail to alloc devno\n");

19 goto alloc_err;

20 }

21 //第二步

22 //关联字符设备结构体cdev与文件操作结构体file_operations

23 cdev_init(&chr_dev, &chr_dev_fops);

24 //第三步

25 //添加设备至cdev_map散列表中

26 ret = cdev_add(&chr_dev, devno, DEV_CNT);

27 if (ret < 0) {

28 printk("fail to add cdev\n");

29 goto add_err;

30 }

31 return 0;

32

33 add_err:

34 //添加设备失败时，需要注销设备号

35 unregister_chrdev_region(devno, DEV_CNT);

36 alloc_err:

37 return ret;

38 }

39 module_init(chrdev_init);

在模块的加载函数中，代码清单
57‑20的第16~20行使用动态分配的方式来获取设备号，指定设备的名称为“EmbedCharDev”，只申请一个设备号，并且次设备号为0。这里使用C语言的goto语法，当获取失败时，直接返回对应的错误码。成功获取到设备号之后，我们还缺字符设备结构体以及文件的操作方式。代码清单 57‑20中使用定义
变量的方式定义了一个字符设备结构体chr_dev，调用cdev_init函数将chr_dev结构体和文件操作结构体相关联，该结构体的具体实现下节见分晓。到这里，我们的字符设备就已经编写完毕。最后我们只需要调用cdev_add函数将我们的字符设备添加到字符设备管理列表cdev_map即可。此处也使用了
goto语法，当添加设备失败的话，需要将申请的设备号注销掉，要养成一个好习惯，不要“占着茅坑不拉屎”。

模块的卸载函数就相对简单一下，只需要完成注销设备号，以及移除字符设备，见代码清单 57‑21。

代码清单 57‑21 内核模块卸载函数（位于文件chrdev.c）

1 static void \__exit chrdev_exit(void)

2 {

3 printk("chrdev exit\n");

4 unregister_chrdev_region(devno, DEV_CNT);

5

6 cdev_del(&chr_dev);

7 }

8 module_exit(chrdev_exit);

文件操作方式的实现
^^^^^^^^^

下面，我们开始实现字符设备最重要的部分：文件操作方式结构体file_operations，见代码清单 57‑22。

代码清单 57‑22 file_operations结构体（位于文件chrdev.c）

1 #define BUFF_SIZE 128

2 //数据缓冲区

3 static char vbuf[BUFF_SIZE];

4 static struct file_operations chr_dev_fops = {

5 .owner = THIS_MODULE,

6 .open = chr_dev_open,

7 .release = chr_dev_release,

8 .write = chr_dev_write,

9 .read = chr_dev_read,

10 };

由于这个字符设备是一个虚拟的设备，与硬件并没有什么关联，因此，open函数与release直接返回0即可，我们重点关注write以及read函数的实现。

代码清单 57‑23 chr_dev_open函数与chr_dev_release函数（位于文件chrdev.c）

1 static int chr_dev_open(struct inode \*inode, struct file \*filp)

2 {

3 printk("\nopen\n");

4 return 0;

5 }

6

7 static int chr_dev_release(struct inode \*inode, struct file \*filp)

8 {

9 printk("\nrelease\n");

10 return 0;

11 }

我们在open函数与release函数中打印相关的调试信息，见代码清单 57‑23。

代码清单 57‑24 chr_dev_write函数（位于文件chrdev.c）

1 static ssize_t chr_dev_write(struct file \*filp, const char \__user \* buf, size_t count, loff_t \*ppos)

2 {

3 unsigned long p = \*ppos;

4 int ret;

5 int tmp = count ;

6 if (p > BUFF_SIZE)

7 return 0;

8 if (tmp > BUFF_SIZE - p)

9 tmp = BUFF_SIZE - p;

10 ret = copy_from_user(vbuf, buf, tmp);

11 \*ppos += tmp;

12 return tmp;

13 }

当我们的应用程序调用write函数，最终就调用我们的chr_dev_write函数。在该函数中，变量p记录了当前文件的读写位置，如果超过了数据缓冲区的大小（128字节）的话，直接返回0。并且如果要读写的数据个数超过了数据缓冲区剩余的内容的话，则只读取剩余的内容。使用copy_from_user从用户
空间拷贝tmp个字节的数据到数据缓冲区中，同时让文件的读写位置偏移同样的字节数。

代码清单 57‑25 chr_dev_read函数（位于文件chrdev.c）

1 static ssize_t chr_dev_read(struct file \*filp, char \__user \* buf, size_t count, loff_t \*ppos)

2 {

3 unsigned long p = \*ppos;

4 int ret;

5 int tmp = count ;

6

7

8 if (p >= BUFF_SIZE)

9 return 0;

10 if (tmp > BUFF_SIZE - p)

11 tmp = BUFF_SIZE - p;

12 ret = copy_to_user(buf, vbuf+p, tmp);

13 \*ppos +=tmp;

14 return tmp;

15 }

同样的，当我们应用程序调用read函数，则会执行chr_dev_read函数的内容。该函数的实现与chr_dev_write函数类似，区别在于，使用copy_to_user从数据缓冲区拷贝tmp个字节的数据到用户空间中。

应用程序验证
^^^^^^

代码清单 57‑26 Makefile

1 KERNEL_DIR=/home/embedfire/module/linux-imx

2

3 obj-m := chrdev.o

4

5 all:

6 $(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) modules

7

8 .PHONY:clean

9 clean:

10 $(MAKE) -C $(KERNEL_DIR) M=$(CURDIR) clean

编写Makefile，执行make，生成的chrdev.ko文件通过nfs网络文件系统，让开发板能够访问该文件。执行以下命令：

insmod chrdev.ko

cat /proc/devices

|charac007|

图 57‑5 EmbedCharDev设备

我们从/proc/devices文件中，可以看到我们注册的字符设备EmbedCharDev的主设备号为248。

mknod /dev/chrdev c 248 0

使用mknod命令来创建一个新的设备chrdev，见图 57‑6。

|charac008|

图 57‑6 chrdev设备

下面，我们开始编写应用程序，来读写我们的字符设备，见代码清单 57‑27。

代码清单 57‑27 main.c函数（位于文件main.c）

1 #include <stdio.h>

2 #include <unistd.h>

3 #include <fcntl.h>

4 #include <string.h>

5 char \*wbuf = "Hello World\n";

6 char rbuf[128];

7 int main(void)

8 {

9 printf("EmbedCharDev test\n");

10 //打开文件

11 int fd = open("/dev/chrdev", O_RDWR);

12 //写入数据

13 write(fd, wbuf, strlen(wbuf));

14 //写入完毕，关闭文件

15 close(fd);

16 //打开文件

17 fd = open("/dev/chrdev", O_RDWR);

18 //读取文件内容

19 read(fd, rbuf, 128);

20 //打印读取的内容

21 printf("The content : %s", rbuf);

22 //读取完毕，关闭文件

23 close(fd);

24 return 0;

25 }

main函数中，打开文件/dev/chrdev，这里只是进行简单的读写测试。最后，我们可以看到终端的输出信息，见图 57‑7。

|charac009|

图 57‑7 实验结果

实际上，我们也可以通过echo或者cat命令，来测试我们的设备驱动程序。

echo "EmbedCharDev test" > /dev/chrdev

cat /dev/chrdev

|charac010|

图 57‑8 echo、cat命令测试结果

当我们不需要该内核模块的时候，我们可以执行以下命令：

rmmod chrdev.ko

rm /dev/chrdev

使用命令rmmod，卸载内核模块，并且删除相应的设备文件。

一个驱动支持多个设备
~~~~~~~~~~

在Linux内核中，主设备号用于标识设备对应的驱动程序，告诉Linux内核使用哪一个驱动程序为该设备服务。但是，次设备号表示了同类设备的各个设备。图 57‑4中列出了所有主设备号为1的设备，每个设备的功能都是不一样的。如何能够用一个驱动程序去控制各种设备呢？很明显，首先，我们可以根据次设备号，来区分
各种设备；其次，就是前文提到过的file结构体的私有数据成员private_data。我们可以通过该成员来做文章，不难想到为什么只有open函数和close函数的形参才有file结构体，因为驱动程序第一个执行的是操作就是open，通过open函数就可以控制我们想要驱动的底层硬件。

下面介绍第一种实现方式，将我们的上一节程序改善一下，生成了两个设备，各自管理各自的数据缓冲区。

代码清单 57‑28 chrdev.c修改部分（位于文件chrdev.c）

1 #define DEV_NAME "EmbedCharDev"

2 #define DEV_CNT (2) (1)

3 #define BUFF_SIZE 128

4 //定义字符设备的设备号

5 static dev_t devno;

6 //定义字符设备结构体chr_dev

7 static struct cdev chr_dev;

8 //数据缓冲区

9 static char vbuf1[BUFF_SIZE]; (2)

10 static char vbuf2[BUFF_SIZE]; (3)

代码清单 57‑28中，（1）处修改了宏定义DEV_CNT，将原本的个数1改为2，这样的话，我们的驱动程序便可以管理两个设备。（2）~（3）处修改为两个数据缓冲区。

代码清单 57‑29 chr_dev_open函数修改（位于文件chrdev.c）

1 static int chr_dev_open(struct inode \*inode, struct file \*filp)

2 {

3 printk("\nopen\n ");

4 switch (MINOR(inode->i_rdev)) {

5 case 0 : {

6 filp->private_data = vbuf1;

7 break;

8 }

9 case 1 : {

10 filp->private_data = vbuf2;

11 break;

12 }

13 }

14 return 0;

15 }

我们知道inode结构体中，对于设备文件的设备号会被保存到其成员i_rdev中。在chr_dev_open函数中，我们使用宏定义MINOR来获取该设备文件的次设备号，使用private_data指向各自的数据缓冲区。对于次设备号为0的设备，负责管理vbuf1的数据，对于次设备号为1的设备，则用于管理
vbuf2的数据，这样就实现了同一个设备驱动，管理多个设备了。接下来，我们的驱动只需要对private_data进行读写即可。

代码清单 57‑30 chr_dev_write函数（位于文件chrdev.c）

1 static ssize_t chr_dev_write(struct file \*filp, const char \__user \* buf, size_t count, loff_t \*ppos)

2 {

3 unsigned long p = \*ppos;

4 int ret;

5 char \*vbuf = filp->private_data;

6 int tmp = count ;

7 if (p > BUFF_SIZE)

8 return 0;

9 if (tmp > BUFF_SIZE - p)

10 tmp = BUFF_SIZE - p;

11 ret = copy_from_user(vbuf, buf, tmp);

12 \*ppos += tmp;

13 return tmp;

14 }

可以看到，我们的chr_dev_write函数改动很小，只是增加了第5行的代码，将原先vbuf数据指向了private_data，这样的话，当我们往次设备号为0的设备写数据时，就会往vbuf1中写入数据。次设备号为1的设备写数据，也是同样的道理。

代码清单 57‑31 chr_dev_read函数（位于文件chrdev.c）

1 static ssize_t chr_dev_read(struct file \*filp, char \__user \* buf, size_t count, loff_t \*ppos)

2 {

3 unsigned long p = \*ppos;

4 int ret;

5 int tmp = count ;

6 char \*vbuf = filp->private_data;

7 if (p >= BUFF_SIZE)

8 return 0;

9 if (tmp > BUFF_SIZE - p)

10 tmp = BUFF_SIZE - p;

11 ret = copy_to_user(buf, vbuf+p, tmp);

12 \*ppos +=tmp;

13 return tmp;

14 }

同样的，chr_dev_read函数也只是增加了第6行的代码，将原先的vbuf指向了private_data成员。

至于Makefile文件，与上一小节的相同，这里便不再罗列出来了。下面我们使用cat以及echo命令，对我们的驱动程序进行测试。

insmod chrdev.ko

mknod /dev/chrdev1 c 248 0

mknod /dev/chrdev2 c 248 1

通过以上命令，加载了新的内核模块，同时创建了两个新的字符设备，分别是/dev/chrdev1和/dev/chrdev2，开始进行读写测试：

echo “hello world” > /dev/chrdev1

echo “123456” > /dev/chrdev2

cat /dev/chrdev1

cat /dev/chrdev2

|charac011|

图 57‑9 实验结果

可以看到设备chrdev1中保存了字符串“hello world”，而设备chrdev2中保存了字符串“123456”。只需要几行代码，就可以实现一个驱动程序，控制多个设备。

我们回忆一下，我们前面讲到的文件节点inode中的成员i_cdev，为了方便访问设备文件，在打开文件过程中，将对应的字符设备结构体cdev保存到该变量中，那么我们也可以通过该变量来做文章。

代码清单 57‑32 定义设备（文件main.c）

1 //虚拟字符设备

2 struct chr_dev {

3 struct cdev dev;

4 char vbuf[BUFF_SIZE];

5 };

6 //字符设备1

7 static struct chr_dev vcdev1;

8 //字符设备2

9 static struct chr_dev vcdev2;

代码清单 57‑32中定义了一个新的结构体struct chr_dev，它有两个结构体成员：字符设备结构体dev以及设备对应的数据缓冲区。使用新的结构体类型struct chr_dev定义两个虚拟设备vcdev1以及vcdev2。

代码清单 57‑33 chrdev_init函数（文件main.c）

1 static int \__init chrdev_init(void)

2 {

3 int ret;

4 printk("4 chrdev init\n");

5 ret = alloc_chrdev_region(&devno, 0, DEV_CNT, DEV_NAME);

6 if (ret < 0)

7 goto alloc_err;

8

9 //关联第一个设备：vdev1

10 cdev_init(&vcdev1.dev, &chr_dev_fops);

11 ret = cdev_add(&vcdev1.dev, devno+0, 1);

12 if (ret < 0) {

13 printk("fail to add vcdev1 ");

14 goto add_err1;

15 }

16 //关联第二个设备：vdev2

17 cdev_init(&vcdev2.dev, &chr_dev_fops);

18 ret = cdev_add(&vcdev2.dev, devno+1, 1);

19 if (ret < 0) {

20 printk("fail to add vcdev2 ");

21 goto add_err2;

22 }

23 return 0;

24 add_err2:

25 cdev_del(&(vcdev1.dev));

26 add_err1:

27 unregister_chrdev_region(devno, DEV_CNT);

28 alloc_err:

29 return ret;

30

31 }

chrdev_init函数的框架仍然没有什么变化。只不过，在添加字符设备时，使用cdev_add依次添加。注意，当虚拟设备1添加失败时，直接返回的时候，只需要注销申请到的设备号即可。若虚拟设备2添加失败，则需要把虚拟设备1移动，再将申请的设备号注销。

代码清单 57‑34 chrdev_exit函数（文件main.c）

1 static void \__exit chrdev_exit(void)

2 {

3 printk("chrdev exit\n");

4 unregister_chrdev_region(devno, DEV_CNT);

5 cdev_del(&(vcdev1.dev));

6 cdev_del(&(vcdev2.dev));

7 }

chrdev_exit函数注销了申请到的设备号，使用cdev_del移动两个虚拟设备。

代码清单 57‑35 chr_dev_open以及chr_dev_release函数（文件main.c）

1 static int chr_dev_open(struct inode \*inode, struct file \*filp)

2 {

3 printk("open\n");

4 filp->private_data = container_of(inode->i_cdev, struct chr_dev, dev);

5 return 0;

6 }

7

8 static int chr_dev_release(struct inode \*inode, struct file \*filp)

9 {

10 printk("release\n");

11 return 0;

12 }

我们知道inode中的i_cdev成员保存了对应字符设备结构体的地址，但是我们的虚拟设备是把cdev封装起来的一个结构体，我们要如何能够得到虚拟设备的数据缓冲区呢？为此，Linux提供了一个宏定义container_of，该宏可以根据结构体的某个成员的地址，来得到该结构体的地址。该宏需要三个参数，分
别是代表结构体成员的真实地址，结构体的类型以及结构体成员的名字。在chr_dev_open函数中，我们需要通过inode的i_cdev成员，来得到对应的虚拟设备结构体，并保存到文件指针filp的私有数据成员中。假如，我们打开虚拟设备1，那么inode->i_cdev便指向了vcdev1的成员dev，
利用container_of宏，我们就可以得到vcdev1结构体的地址，也就可以操作对应的数据缓冲区了。

代码清单 57‑36 chr_dev_write函数（文件main.c）

1 static ssize_t chr_dev_write(struct file \*filp, const char \__user \* buf, size_t count, loff_t \*ppos)

2 {

3 unsigned long p = \*ppos;

4 int ret;

5 //获取文件的私有数据

6 struct chr_dev \*dev = filp->private_data;

7 char \*vbuf = dev->vbuf;

8

9 int tmp = count ;

10 if (p > BUFF_SIZE)

11 return 0;

12 if (tmp > BUFF_SIZE - p)

13 tmp = BUFF_SIZE - p;

14 ret = copy_from_user(vbuf, buf, tmp);

15 \*ppos += tmp;

16 return tmp;

17 }

对比第一种方法，实际上只是新增了第6行代码，通过文件指针filp的成员private_data得到相应的虚拟设备。修改第7行的代码，定义了char类型的指针变量，指向对应设备的数据缓冲区。

代码清单 57‑37 chr_dev_read函数（文件main.c）

1 static ssize_t chr_dev_read(struct file \*filp, char \__user \* buf, size_t count, loff_t \*ppos)

2 {

3 unsigned long p = \*ppos;

4 int ret;

5 int tmp = count ;

6 //获取文件的私有数据

7 struct chr_dev \*dev = filp->private_data;

8 char \*vbuf = dev->vbuf;

9 if (p >= BUFF_SIZE)

10 return 0;

11 if (tmp > BUFF_SIZE - p)

12 tmp = BUFF_SIZE - p;

13 ret = copy_to_user(buf, vbuf+p, tmp);

14 \*ppos +=tmp;

15 return tmp;

16 }

读函数，与写函数的改动部分基本一致，这里就只贴出代码，不进行讲解。

|charac012|

图 57‑10 实验结果

我们往两个数据缓冲区分别写入“HelloWorld”以及“DemoTest”字符串，然后使用cat命令来读取设备，实验结果见图 57‑10。

总结一下，一个驱动支持多个设备的具体实现方式的重点在于如何运用file的私有数据成员。第一种方法是通过将各自的数据缓冲区放到该成员中，在读写函数的时候，直接就可以对相应的数据缓冲区进行操作；第二种方法则是通过将我们的数据缓冲区和字符设备结构体封装到一起，由于文件结构体inode的成员i_cdev保存
了对应字符设备结构体，使用container_of宏便可以获得封装后的结构体的地址，进而得到相应的数据缓冲区。

到这里，字符设备驱动就已经讲解完毕了。如果你在阅读57.2 时，发现自己有好多不理解的地方，学完本章之后，建议重新梳理一下整个过程，有助于加深对整个字符设备驱动框架的理解。

.. |charac002| image:: media/charac002.jpg
   :width: 5.6482in
   :height: 2.26319in
.. |charac003| image:: media/charac003.jpg
   :width: 5.76806in
   :height: 1.63046in
.. |charac004| image:: media/charac004.jpg
   :width: 5.76806in
   :height: 2.29444in
.. |charac005| image:: media/charac005.jpg
   :width: 3.70833in
   :height: 2.04167in
.. |charac006| image:: media/charac006.jpg
   :width: 5.76806in
   :height: 1.99375in
.. |charac007| image:: media/charac007.jpg
   :width: 3.325in
   :height: 4.14167in
.. |charac008| image:: media/charac008.jpg
   :width: 3.75in
   :height: 0.31667in
.. |charac009| image:: media/charac009.jpg
   :width: 2.45in
   :height: 1.59167in
.. |charac010| image:: media/charac010.jpg
   :width: 4.90833in
   :height: 1.56667in
.. |charac011| image:: media/charac011.jpg
   :width: 5.76806in
   :height: 3.06458in
.. |charac012| image:: media/charac012.jpg
   :width: 5.65833in
   :height: 3.33333in
