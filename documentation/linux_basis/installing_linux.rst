.. vim: syntax=rst

安装Ubuntu系统
--------------------

要进行Linux开发，自然少不了一台运行Linux系统的开发主机。从PC上开始使用Linux系统是最好的熟悉方法。

对于熟悉Linux且有多余主机的用户，建议直接采用独立的主机安装系统作为开发环境。而对于入门级用户，强烈建议在Windows环境下安装VirtualBox或VMware虚拟机软件，然后在虚拟机上安装相应的Linux系统。在虚拟机上体验Linux系统的好处是没有后顾之忧，觉得不如意就推倒重来，而且依旧
可以使用Windows进行其它工作，也方便在Windows与Ubuntu虚拟机之间进行文件共享。即使是只进行Linux嵌入式开发工作，也避免不了需要使用Windows下的工具，如后期我们给开发板烧录uboot、Linux内核时，使用到的NXP官方MFGtools工具时，还是依赖于Windows系统。

安装虚拟机VirtualBox
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

虚拟机技术是虚拟化技术的一种，所谓虚拟化技术就是将事物从一种形式转变成另一种形式，最常用的虚拟化技术有操作系统中内存的虚拟化，实际运行时用户需要的内存空间可能远远大于物理机器的内存大小，利用内存的虚拟化技术，用户可以将一部分硬盘虚拟化为内存，而这对用户是透明的。

流行的虚拟机软件有VMware、Virtual Box和Virtual PC，它们都能在Windows系统上虚拟出多个计算机，本书使用的是免费的虚拟机软件Virtual Box，我们可以去\ https://www.virtualbox.org/wiki/Downloads\
官网中下载VirtualBox软件，也可以使用我们提供的资料光盘中的软件，进行安装。

|instal002|

图 4‑1 官网下载地址

下面介绍一下，安装虚拟机软件VirtualBox的全过程：

(1) 运行VirtualBox-6.0.2-128162-Win.exe，可以看到的界面，点击“下一步”，见图 4‑2。

|instal003|

图 4‑2 安装向导

(2) 需要安装的功能，选择默认即可。软件的安装路径，用户可以根据实际情况自由选择，本书选择安装在E盘，点击“下一步”，见图 4‑3。

|instal004|

图 4‑3 选择安装路径

(3) 安装过程，选择软件自定义的功能，这里选择默认即可，点击“下一步”，见图 4‑4。

|instal005|

图 4‑4 自定义安装

(4) 在安装过程中，会提示一个警告，我们可以忽略该警告，点击“是”，继续安装，见图 4‑5。

|instal006|

图 4‑5 安装警告

(5) 图 4‑6，点击“安装”按钮，开始安装软件。

|instal007|

图 4‑6 开始安装

(6) 这样之后，便开始安装VirtualBox软件，静静地等待，软件安装完成。

|instal008|

图 4‑7 安装过程

(7) 软件安装成功，点击“完成”，运行VirtualBox，见图 4‑8。

|instal009|

图 4‑8 安装成功

|instal010|

图 4‑9 软件VirtualBox

获取Ubuntu系统
~~~~~~~~~~~~~~~~~~~~~~~~~~

本书以Ubuntu 18.04桌面发行版本作为开发主机的主体系统环境，对于后期编译内核等依赖不同版本系统环境的情况，我们会采用Docker技术创建其它运行环境。

在安装前需要先到Ubuntu的官网下载桌面版的系统镜像，可在如下网址下载得到：\ https://ubuntu.com/download/desktop\ ，具体见图 4‑10。

|instal011|

图 4‑10 Ubuntu下载页面

使用VirtualBox安装Ubuntu系统
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

以下为使用VirtualBox安装Ubuntu18.04系统的操作步骤。在以下的步骤中，可按照自己使用的电脑参数进行配置

(1) 可从以下地址获取VirtualBox软件：\ https://www.virtualbox.org/wiki/Downloads\ ，下载后直接安装即可。

(2) 运行VirtualBox软件，点击“新建”按钮，来创建一个新的虚拟机，如图 4‑11所示。点击“下一步”。

|instal012|

图 4‑11 创建虚拟机

(3) 选择内存大小，默认为1024MB，可以根据自己电脑的配置选择，这里选择4096MB，即4GB的内存大小，点击“下一步”按钮。

|instal013|

图 4‑12 分配内存大小

(4) 选择虚拟硬盘，如图 4‑13所示，选择“现在创建虚拟硬盘”，点击“创建”。出现图 4‑14所示的窗口，选择虚拟硬盘的类型，一般情况都选择VDI，单击“下一步”。如图 4‑15所示，虚拟硬盘的分配方式有两种：一种是动态分配，需要多少用多少，一开始占用小，随着空间需要而增长；另一种是固定大小，它
会直接占用所分配的物理硬盘空间，但是访问速度快。这里我们选择“动态分配”，点击“下一步”之后，就需要我们选择虚拟硬盘大小，默认是10GB，用户可以根据自己电脑配置和需要进行分配，这里选择100GB（它不会立即占用这么多的空间，根据自己需求设置即可）。如图
4‑16所示，最后点击“创建”按钮，这样就创建好了，如图 4‑17所示。

|instal014|

图 4‑13 创建虚拟硬盘

|instal015|

图 4‑14 选择虚拟硬盘类型

|instal016|

图 4‑15 虚拟硬盘的分配方式

|instal017|

图 4‑16 指定虚拟硬盘的大小

|instal018|

图 4‑17 创建成功

(5) 点击右侧的“设置”按钮，如图 4‑18所示，选择“存储”栏目，然后选择处来设置系统镜像文件，通过点击处的按钮来选择我们Ubuntu镜像文件（*.iso）的路径，设置成功之后，如图 4‑19所示，点击“OK”，进入下一步。

|instal019|

图 4‑18 设置镜像文件

|instal020|

图 4‑19 镜像文件设置成功

(6) 选择“网络”栏目，将网卡连接方式配置为桥接网卡，如图 4‑20所示。

|instal021|

图 4‑20 设置网卡连接方式

(7) 安装Ubuntu18.04系统，点击启动“启动”按钮，启动虚拟机，如图 4‑21所示。进入安装程序，如图 4‑22所示。

|instal022|

图 4‑21 启动虚拟机

|instal023|

图 4‑22 安装程序

(8) 选择语言，处可以选择英文，中文等语言，这里我们选择 “简体中文”，然后点击处的“install Ubuntu”按钮，进入下一步。

..

   |instal024|

(9) 选择键盘布局，如图 4‑23，点击“继续”。

|instal025|

图 4‑23 键盘布局

(10) 选择一些软件安装，如图 4‑24所示，点击“继续”，进入下一步。

|instal026|

图 4‑24 软件更新

(11) 进入如图 4‑25所示的界面，由于虚拟机的虚拟硬盘是独占的，我们选择默认的“清除整个磁盘并安装Ubuntu”即可。

|instal027|

图 4‑25 选择安装类型

(12) 点击继续后会弹出提示确认磁盘的分区修改，点击继续即可。

|instal028|

图 4‑26 确认磁盘分区

(13) 选择时区，在图 4‑27提示界面区域点击我国东侧位置，其下的文本会出现“Shanghai”字样，选择后系统使用的就是北京时间。

..

   |instal029|

图 4‑27选择默认时区

(14) 进入如图 4‑28所示的界面，在这个界面中输入用户名和密码，设置计算机名称，用户可以根据自己的喜好来设置。此处设置的用户会默认具有系统管理员的权限，使用Linux系统时，常常需要使用该密码进行认证，请牢记该密码。

|instal030|

图 4‑28 配置用户名和密码

(15) 如图 4‑29所示，等待安装完成，然后重启虚拟机，输入密码，就可以看到Linux的桌面了，如图 4‑30所示。

|instal031|

图 4‑29 正在安装系统

|instal032|

图 4‑30 安装成功

给虚拟机安装增强功能
~~~~~~~~~~~~~~~~~~~~~~~~~~

安装完成操作系统后，可给虚拟机安装增强功能，安装后支持双向复制、共享文件、屏幕自动调整分辨率，使用起来更加方便。

安装过程如下：

(1) 在虚拟机界面点击“设备/安装增强功能”即可，见图 4‑31。

|instal033|

图 4‑31 点击安装增强功能

(2) 点击安装增强功能后，虚拟机中会插入一个光盘，并会自动启动安装程序，见图 4‑32，在弹出的界面点击运行。若没有自动启动，可手动双击该光盘进行安装。

|instal034|

图 4‑32 运行安装程序

(3) 点击运行后会出现终端界面输出运行提示，最后提示“press return to close this window”时，按回车键退出即可。

|instal035|

图 4‑33 安装完成

(4) 重启虚拟机，进入系统后尝试调整虚拟机控制界面的窗口大小，看到虚拟机内桌面根据窗口大小调整分辨率表明安装成功。

(5) 设置虚拟机控制选项中的“设备/共享粘贴板”和“设备/拖放”一栏可以设置虚拟机与主机之间的粘贴板和拖放功能。

设置共享文件夹
~~~~~~~~~~~~~~~

在未来的学习过程中，我们有时候需要把下载的资料文件放到虚拟机中，或者是需要把虚拟机的资料挪到Windows主机中，常用的解决方法有很多，如winscp，FileZilla等软件。不过，上述的软件虽然在使用的过程十分快捷方便，但是在安装过程中，都相当地麻烦，一会需要在虚拟机上操作，一会又要到主机上操作
。我们的虚拟机实际上提供了一个强大的功能：共享文件夹。我们可以通过这个功能，实现主机与虚拟机的文件传输，安装方式以及使用方法也都十分简单。下面，介绍一下如何开启虚拟机VirtualBox的共享文件夹功能（执行以下步骤时，确保Linux虚拟机处于关机状态）。

(1) 新建文件夹，用于Windows主机和Linux虚拟机共享文件夹。用户可以根据实际情况，创建共享文件夹，见图 4‑34。

|instal036|

图 4‑34 新建文件夹

(2) 设置虚拟机的共享文件夹，点击图 4‑35的处，弹出设置窗口，选择“共享文件夹”选项卡（图 4‑35的处），最后点击的按钮，添加共享文件夹，进入下一步。

|instal037|

图 4‑35 设置共享文件夹

(3) 图 4‑36中，点击处按钮，新增共享文件夹，在弹出的“添加共享文件夹”窗口，我们可以看到处有一个下拉箭头，我们选中处，来选择我们刚刚新建的文件夹。到这里之后，我们就完成了文件夹路径的设置。我们仍然需要让虚拟机启动的时候，自动挂载共享文件夹，见图
4‑37。选中“自动挂载”选项，在处填入共享文件夹的挂载点，这里我们建议初学者使用我们提供的路径：

/home/用户名/ebf_dir

   用户名需要根据用户安装系统时，输入的用户名。本书使用的embedfire，因此，我们输入的路径为“/home/embedfire/ebf_dir”，最后点击“OK”按钮即可完成设置，见图 4‑37。

|instal038|

图 4‑36 设置共享文件夹

|instal039|

图 4‑37 设置自动挂载

|instal040|

图 4‑38 设置成功

(4) 启动虚拟机，我们就可以看到我们的共享文件夹了，见图 4‑39。

..

   |instal041|

图 4‑39共享文件夹

(5) 单击鼠标右键，左键点击“打开终端”，见图 4‑40。

|instal042|

图 4‑40 打开终端

出现图 4‑41的窗口，输入命令：

sudo usermod -a -G vboxsf 用户名

本机的用户名是embedfire，因此，输入命令“sudo usermod –a –G vboxsf embedfire”，见图 4‑42。

|instal043|

图 4‑41 终端terminal窗口

|instal044|

图 4‑42 添加用户到某个用户组

(6) 打开之后，这里为了更好地说明，因此在Windows下新建一个普通的txt文档，我们可以看到Linux也可以对该文档进行操作。

..

   |instal045|

图 4‑43 共享文件

熟悉系统
~~~~~~~~~~~~

Ubuntu系统安装好后，请随意体验一下Ubuntu系统，看看能用它做些什么日常操作。

它自带有浏览器，音乐播放器以及一些与Office功能类似的办公软件，如果安装的是中文版，中文输入法也会默认被配置好。建议使用打开浏览器看看自己的常用网站，打开系统的文件夹新建文件随便记录一些内容，甚至安装一下Steam游戏平台，看看能玩什么游戏。对于与Windows系统的差异，如不能使用MCU的开发
软件Keil等，不能用Adobe的PS等软件，在这些方面就不要去强求和折腾。现在国产软件开始对Linux重新重视了起来，腾讯QQ也于2019-10-24回归Linux了。

应用列表
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

下面我们以Ubuntu下使用gedit编辑器编写文件为例，让大家去熟悉操作系统。在Ubuntu桌面的左下角有个应用软件列表按钮，点开后可看到系统中包含的应用，如图 4‑44。

|instal046|

图 4‑44 应用软件列表

图 4‑44中的“文本编辑器”即是Ubuntu系统自带的gedit编辑器，直接点击后打开可以输入文字，它的使用就类似Windows系统自带的记事本软件一样。

使用拼音输入法
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

如果安装Ubuntu系统时选择了中文支持，那么系统安装后就自带拼音输入法，其设置位置在桌面状态栏的“zh”图标中，点击后可选择汉语拼音输入法，如图 4‑45。

|instal047|

图 4‑45 使用拼音输入法

|instal048|

图 4‑46 文字输入

该输入法使用效果如图 4‑46，使用输入法时可以通过“Shift”键快速切换中英文输入。

在后面我们使用命令行的时候，建议直接把输入法关闭掉，即重新点击输入法设置的图标，把它选择回“zh”即可。

文件浏览器
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

输入完内容后点击编辑器右侧的保存按钮，它会弹出选择文件保存位置的弹框，如图 4‑47。

|instal049|

图 4‑47 保存文件

可以看到它默认的保存位置是“主目录embedfire”，这个是用户自己的目录，如果你选择其它位置，有可能会因为没有权限而无法保存。选定好存储位置并输入文件名称后，通过保存按钮可保存文件。

保存关闭文件后，点击桌面任务栏的文件浏览器图标，可以打开到刚刚文件存放的目录，查找到该文件，如图 4‑48。

|instal050|

图 4‑48文件浏览器

安装软件及权限
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

不同的Linux发行版安装应用软件的方式不尽相同，Ubuntu系统自带了软件中心，使用它可以非常方便地安装和卸载各种软件。

在桌面的任务栏有“Ubuntu软件”图标，图 4‑49所示，点击后可以打开软件中心。

|instal051|

图 4‑49 Ubuntu软件

我们尝试安装“2048”小游戏。打开软件之后，点击搜索的按钮（图 4‑50中的框框处），输入“2048”，如图 4‑51所示，点击安装框框处的“2048”游戏。

|instal052|

图 4‑50 Ubuntu软件中心全貌

|instal053|

图 4‑51小游戏2048

安装软件时，可能会弹出如图 4‑52的提示，由于需要系统管理员的权限，所以要输入密码进行认证，该密码为安装系统时，图 4‑28中设置的用户密码。

|instal054|

图 4‑52 认证提示

在Linux系统下，所有的操作都有明确的权限要求。如安装软件需要系统管理员权限；普通用户在自己所属的目录下才能创建文件等。

目前Ubuntu软件中心采用Snap软件市场来安装软件，暂时还没有国内的镜像下载源，所以下载可能需要花比较长的时间。如果不想体验可以直接点击取消安装。

.. |instal002| image:: media/instal002.jpeg
   :width: 2.77273in
   :height: 1.53788in
.. |instal003| image:: media/instal003.jpg
   :width: 5.15in
   :height: 4.025in
.. |instal004| image:: media/instal004.jpg
   :width: 5.15in
   :height: 4.025in
.. |instal005| image:: media/instal005.jpg
   :width: 5.15in
   :height: 4.025in
.. |instal006| image:: media/instal006.jpg
   :width: 5.15in
   :height: 4.025in
.. |instal007| image:: media/instal007.jpg
   :width: 5.15in
   :height: 4.025in
.. |instal008| image:: media/instal008.jpg
   :width: 5.15in
   :height: 4.025in
.. |instal009| image:: media/instal009.jpg
   :width: 5.15in
   :height: 4.025in
.. |instal010| image:: media/instal010.jpeg
   :width: 5.29189in
   :height: 2.83899in
.. |instal011| image:: media/instal011.png
   :width: 5.76806in
   :height: 2.68709in
.. |instal012| image:: media/instal012.jpg
   :width: 4.46903in
   :height: 2.74835in
.. |instal013| image:: media/instal013.jpg
   :width: 4.30833in
   :height: 3.68333in
.. |instal014| image:: media/instal014.jpg
   :width: 4.30833in
   :height: 3.68333in
.. |instal015| image:: media/instal015.jpg
   :width: 4.36667in
   :height: 4.15in
.. |instal016| image:: media/instal016.jpg
   :width: 4.36667in
   :height: 4.15in
.. |instal017| image:: media/instal017.jpg
   :width: 4.36667in
   :height: 4.15in
.. |instal018| image:: media/instal018.jpg
   :width: 5.76806in
   :height: 3.54722in
.. |instal019| image:: media/instal019.jpg
   :width: 5.76806in
   :height: 3.76984in
.. |instal020| image:: media/instal020.jpg
   :width: 5.76806in
   :height: 3.68542in
.. |instal021| image:: media/instal021.jpg
   :width: 5.76806in
   :height: 3.68542in
.. |instal022| image:: media/instal022.jpg
   :width: 5.76806in
   :height: 3.54722in
.. |instal023| image:: media/instal023.jpg
   :width: 4.75in
   :height: 4.08662in
.. |instal024| image:: media/instal024.jpg
   :width: 5.76806in
   :height: 4.10972in
.. |instal025| image:: media/instal025.jpg
   :width: 5.76806in
   :height: 4.48056in
.. |instal026| image:: media/instal026.jpg
   :width: 5.76806in
   :height: 4.50208in
.. |instal027| image:: media/instal027.jpg
   :width: 4.89103in
   :height: 3.48486in
.. |instal028| image:: media/instal028.jpg
   :width: 5.76806in
   :height: 4.32604in
.. |instal029| image:: media/instal029.jpg
   :width: 4.75in
   :height: 3.39637in
.. |instal030| image:: media/instal030.jpg
   :width: 5.18608in
   :height: 4.50364in
.. |instal031| image:: media/instal031.jpg
   :width: 5.76806in
   :height: 4.34167in
.. |instal032| image:: media/instal032.jpeg
   :width: 5.76806in
   :height: 3.09444in
.. |instal033| image:: media/instal033.png
   :width: 3.99671in
   :height: 2.57051in
.. |instal034| image:: media/instal034.png
   :width: 5.36842in
   :height: 1.88889in
.. |instal035| image:: media/instal035.png
   :width: 4.46154in
   :height: 3.01749in
.. |instal036| image:: media/instal036.jpg
   :width: 5.76806in
   :height: 3.18542in
.. |instal037| image:: media/instal037.jpg
   :width: 4.89394in
   :height: 4.15095in
.. |instal038| image:: media/instal038.jpg
   :width: 5.76806in
   :height: 3.68958in
.. |instal039| image:: media/instal039.jpg
   :width: 2.9in
   :height: 2.28333in
.. |instal040| image:: media/instal040.jpg
   :width: 5.76806in
   :height: 3.68542in
.. |instal041| image:: media/instal041.jpg
   :width: 1.19167in
   :height: 0.80833in
.. |instal042| image:: media/instal042.jpg
   :width: 2.06667in
   :height: 1.81667in
.. |instal043| image:: media/instal043.jpg
   :width: 5.77083in
   :height: 1.22222in
.. |instal044| image:: media/instal044.jpg
   :width: 3.87121in
   :height: 0.2197in
.. |instal045| image:: media/instal045.jpg
   :width: 5.76806in
   :height: 3.14861in
.. |instal046| image:: media/instal046.png
   :width: 3.84057in
   :height: 3.85897in
.. |instal047| image:: media/instal047.png
   :width: 5.38462in
   :height: 2.53688in
.. |instal048| image:: media/instal048.png
   :width: 5.45524in
   :height: 1.95657in
.. |instal049| image:: media/instal049.png
   :width: 5.02308in
   :height: 3.41667in
.. |instal050| image:: media/instal050.png
   :width: 5.40283in
   :height: 1.86538in
.. |instal051| image:: media/instal051.jpg
   :width: 0.41693in
   :height: 1.96154in
.. |instal052| image:: media/instal052.jpg
   :width: 5.76923in
   :height: 3.25in
.. |instal053| image:: media/instal053.png
   :width: 6.15116in
   :height: 3.71745in
.. |instal054| image:: media/instal054.png
   :width: 3.62199in
   :height: 2.59091in
