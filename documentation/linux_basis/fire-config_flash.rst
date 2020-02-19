.. vim: syntax=rst

fire-config刷机
-------------------

1、开发板正常启动后，在串口终端登录debian系统。

.. code-block:: sh
   :emphasize-lines: 2
   :linenos:

   账户:debian
   密码:temppwd

系统正常登录界面如下所示:

.. image:: media/debian_login.png
   :align: center
   :alt: 登录debian系统


2、执行sudo fire-config，选择"flasher"项。

如下图所示:

.. image:: media/fire-config_flasher.png
   :align: center
   :alt: fire-config刷机


3、系统提示: "Would you like the flasher to be enabled?"，选择<Yes>。

如下图所示:

.. image:: media/fire-config_flasher1.png
   :align: center
   :alt: fire-config刷机

3、系统提示: "The flasher is enabled"，选择<OK>。

如下图所示:

.. image:: media/fire-config_flasher2.png
   :align: center
   :alt: fire-config刷机

4、返回fire-config初始界面,选择<Finish>。

如下图所示:

.. image:: media/fire-config_flasher3.png
   :align: center
   :alt: fire-config刷机

5、系统提示:Would you like to reboot now?，选择<Yes>。

如下图所示:

.. image:: media/fire-config_flasher4.png
   :align: center
   :alt: fire-config刷机

6、系统自动重启。

如下图所示:

.. image:: media/fire-config_flasher5.png
   :align: center
   :alt: fire-config刷机

7、重启后的系统将自动进行eMMC或者nandflash刷机。

如下图所示(以eMMC刷机为例):

.. image:: media/fire-config_flasher6.png
   :align: center
   :alt: fire-config刷机

8、耐心等待刷机结束，刷机过程大约持续2分30秒左右。刷机完成后，控制台会重新进入串口登录页面，
此时观察开发板的LED灯，如果LED在持续闪烁，说明刷机成功。

如下图所示

.. image:: media/fire-config_flasher7.png
   :align: center
   :alt: fire-config刷机

9、无论是eMMC，还是nandflash，刷机成功后，调整拨码开关，重新上电启动系统即可。