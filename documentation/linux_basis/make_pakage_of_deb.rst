.. vim: syntax=rst

Linux制作deb包的方法
--------

上一章节我们介绍了如何在开发板出厂配套的系统镜像中使用一些脚本来控制板载外设，为了方便管理这些脚本，
我们将这些脚本都制作在一个deb包中，本章节将介绍如何制作一个deb包，制作deb的方式很多，如使用dpkg-deb方式、
使用checkinstall方式、使用dh_make方式及修改原有的deb包，由于完全从新制作deb包比较复杂，
所以本次介绍的deb包制作方式为修改原有的deb包，本次将基于peripheral_0.1.4_armhf.deb做版本更新。


什么是deb包？
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

deb包是在linux系统下的一种安装包，有时我们在网上下载的Linux软件安装包也会以deb包的形式出现，
由于它是基于tar包的，所以同样会记录着文件的权限信息（读、写、可执行）、所有者、用户组等。


deb包的组成结构
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

deb包一般分成两部分：

- 控制信息（放在DEBIAN目录下），本deb包的DEBIAN目录下有changelog、control、copyright、postinst等文件。changelog文件记录了该deb包的作者、版本以及最后一次更新日期等信息；control文件记录了包名、版本号、架构、维护者及描述等信息；copyright文件记录了一些版权信息；postinst记录了在安装deb包后的欢迎词信息。
- 安装的内容，这部分类似linux的根目录，如根目录下的/home/debian/peripheral存放着我们的运行脚本。


修改已有deb包
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

创建目录结构
^^^^^^^^^^^^^^^^^^^^

创建deb包做需的目录结构，使用mkdir创建一个存放deb包的目录，目录名字可自己取，我创建的目录为peripheral_0.1.7_armhf，接着在peripheral_0.1.7_armhf目录下
下创建DEBIAN目录，如下图所示：

.. image:: media/mk_deb001.PNG
   :align: center
   :alt: 未找到图片01|

接着使用dpkg -X {要解压的deb包路径} {刚刚创建的peripheral_0.1.7_armhf目录} 来解压deb包，解压成功后输出如下信息。

.. image:: media/mk_deb002.PNG
   :align: center
   :alt: 未找到图片02|

上面只是解压了安装的内容，接着使用dpkg -e {要解压的deb包路径} 解压出控制信息（注意要在你创建的目录下解压），如下图所示：

.. image:: media/mk_deb003.PNG
   :align: center
   :alt: 未找到图片03|

从上图可以看到DEBIAN目录下已经有了文件，表明控制信息解压成功。然后就可以修改其中的内容了，修改前的内容如下图所示：

.. image:: media/mk_deb004.PNG
   :align: center
   :alt: 未找到图片04|

修改内容
^^^^^^^^^^^^^^^^^^^^

接着我们往/peripheral_0.1.7_armhf/home/debian/peripheral目录下添加一些硬件测试脚本，添加ap3216c.sh、ds18b20.sh、dht11.sh等脚本，
修改后的内容如下图所示：

.. image:: media/mk_deb005.PNG
   :align: center
   :alt: 未找到图片05|

修改完安装内容后，我们再回到DEBIAN目录下，记录一下本次修改的日期及更新版本号等，比如修改changelog和control文件的信息，如图所示：

.. image:: media/mk_deb006.PNG
   :align: center
   :alt: 未找到图片06|

.. image:: media/mk_deb007.PNG
   :align: center
   :alt: 未找到图片07|

画横线的表示修改过的内容，只修改了版本号和本次修改的日期。

重新打包
^^^^^^^^^^^^^^^^^^^^

最后使用dpkg -b {创建的peripheral_0.1.7_armhf目录} {要打包的包名} 命令便可将修改后的目录重新打成deb包，如图下所示：

.. image:: media/mk_deb008.PNG
   :align: center
   :alt: 未找到图片08|

