.. vim: syntax=rst

运行开发板与串口终端登录
------------------------------------------

本章主要讲解如何使用配套的开发板，让开发板的系统运行起来！

连接网络
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

默认主机名与IP
^^^^^^^^^^^^^^^^^^^^^
SSH终端通过以太网或WiFi都可以连接，连接时可使用主机名或IP地址。

- 开发板的主机名为： **npi** 。
- IP默认由路由动态分配，可先通过串口终端连接，使用 **ifconfig命令** 查看具体IP。


在前面的基础上，需要的额外硬件准备：

-  网线

-  路由器

使用网线通过开发板的任意一个以太网接口与路由器连接起来，即可把开发板连接至网络。

连接外网测试
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

若开发板连接的路由器支持连接到公网，直接在终端输入如下命令即可进行连接测试：



.. code-block:: sh
   :linenos:

   #在开发板的终端执行以下命令
   ping www.firebbs.cn


.. image:: media/boards008.png
   :align: center
   :alt: 未找到图片




出现类似上图的输出表示网络连接成功，使用“Ctrl + C”可退出命令。

ping路由测试
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

若连接的路由不支持连接到公网，可以直接ping路由的IP地址，如本例子中的路由IP地址为192.168.0.1。请把以
下命令中的IP地址换成自己实验环境的路由IP：

.. code-block:: sh
   :linenos:

   ping 192.168.0.1

查看开发板的IP地址
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

如果使用无法ping通，请使用ifconfig命令查看开发板的IP地址，见下图。

.. image:: media/boards009.png
   :align: center
   :alt: 未找到图片




在上图中开发板使用ifconfig命令可查看到开发板eth1网口的IP地址，为192.168.0.69。若
使用ifconfig命令查看不到IP地址，请检查网络连接，或尝试使用以下命令申请IP：

.. code-block:: sh
   :linenos:

   #eth1表示第一路以太网口，eth2表示第二路以太网口
   sudo udhcpc  -b  -i  eth1  

若命令执行正常，会出现图 11-9中的输出，并且再次输入ifconfig命令会查看到eth1具有正常的IP地址。

.. image:: media/boards010.png
   :align: center
   :alt: 未找到图片


修改启动脚本
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

系统在启动时会在显示屏中绘制进度条，这就是 ``/opt/scripts/boot/psplash.sh`` 启动脚本要干的事情，当成功加载完系统后，接着会执行 ``/opt/scripts/boot/psplash_quit.sh`` 启动脚本，那么可以在这个启动脚本中处理一些自己的事情。

比如野火在绘制完进度条后启动qt
app，在 ``/opt/scripts/boot/psplash_quit.sh`` 启动脚本就是这样子写的：

.. code:: bash

    if [ -f /home/debian/qt-app-static/run.sh ] ; then
        sudo /home/debian/qt-app-static/run.sh &
    else
        sudo /home/debian/qt-app/run.sh &
    fi

如果你不想启动qt
app，则可以把上述代码注释掉即可，如果你想在启动后有一个固定的IP地址，那么也可以这样子做， ``xxx`` 改为你自己需要设置的IP地址即可：

.. code:: bash

    sudo ifconfig eth1 down
    sudo ifconfig eth1 192.168.xxx.xxx up

如果你想执行其他操作，就在这里修改即可，此处仅是做个提示。

那么如果我不想等待系统加载完成就运行我的脚本，如何做到呢，其实还是只需要修改 ``/opt/scripts/boot/psplash.sh`` 启动脚本即可，在这个启动脚本中有执行绘制进度条的操作，在改脚本的最后一行，具体如下:

.. code:: bash

    /usr/bin/psplash

那么你也可以在这个启动脚本中添加你需要执行的脚本即可， **请注意：需要脚本所在的绝对路径** 。



