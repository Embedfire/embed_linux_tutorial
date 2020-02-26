.. vim: syntax=rst

使用脚本测试硬件
--------

为方便对开发板的硬件进行测试，我们在开发板出厂配套的系统镜像中提供了一些脚
本演示如何控制板载外设。使用这些脚本可以方便地体验开发板的强大之处，小则可以花样点灯，大则可以播视频音乐。

作为一个合格的程序员，可能在阅读本节的时候，会思考这部分的代码是如何实现的
呢？不过本章我们先进行体验，建立概念，在后面章节再对其中的原理进行详细讲解。只要耐心地
看完本书，精彩都是在后头的。

测试脚本目录
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

测试脚本都被存放在开发板的“/home/root/peripheral”目录，为方便讲解，本章
后面默认是在该目录下进行，先使用如下命令切换至该目录：

.. code-block:: sh
   :linenos:

   cd ~/peripheral




.. image:: media/script002.png
   :align: center
   :alt: 未找到图片2|



如上图是开发板提供的外设示例脚本，不同的开发板提供的内容可能会不同。

LED灯测试
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

led.sh脚本可以控制开发板上的LED灯，对于EBF6ULL Pro开发板的RGB彩灯会进行
混色，对于EBF6ULL Mini开发板三盏用户LED会以不同形式进行闪烁。

在脚本目录下，使用如下命令可直接运行：

#在脚本目录下运行

.. code-block:: sh
   :linenos:

   ./led.sh

对脚本内容感兴趣的用户可以使用cat命令来查看脚本的内容，如下图所示。

.. image:: media/script003.png
   :align: center
   :alt: 未找到图片3|



脚本的内容看似复杂，其本质就是相当于手动在终端中执行echo命令修改LED灯的设备文件。其余脚本大都是使
用类似原理对硬件外设进行控制的。

输入设备测试
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

input.sh脚本可用于对输入设备进行测试，包括按键、触摸屏、鼠标和键盘等。运行脚本可以看到如下图所示 按键
测试所示的终端信息，若想要退出测试时，请按键盘上的“Ctrl”+“c”结束当前进程。

#在脚本目录下运行

.. code-block:: sh
   :linenos:

   ./input.sh

.. image:: media/script004.png
   :align: center
   :alt: 未找到图片4|



该脚本其实是直接调用了《13.4 检测按键》章节提到的evtest命令，若已了解则使用相同的方式测试即可。

执行脚本后，它会列出可用的输入设备，其中右侧说明带“powerkey”或“gpio-keys”字样的
分别对应开发板的“ON/OFF”键及“KEY”键，输设备对应的数字，就可以进行测试。

上图的示例中数字3表示的是KEY按键，若开发板接入了不同的输入设备，对应的数字可能
不同，请按照自己开发板的提示输出进行选择。

选择了KEY键进行测试后，按下开发板对应的KEY键，终端会有相应的事件检测输出。

蜂鸣器
~~~~~~~~~~~~~

EBF6ULL Pro开发板带有蜂鸣器，可进行蜂鸣器测试。

测试时可使用beep.sh脚本。

#在脚本目录下运行

.. code-block:: sh
   :linenos:

   ./beep.sh

执行脚本后，按提示输入1，蜂鸣器则会发出响声，输入0，则会关闭蜂鸣器。

.. image:: media/script005.jpg
   :align: center
   :alt: 未找到图片5|



ADC数据采集
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EBF6ULL Pro开发板带有电位器，可进行ADC电压采集测试。

在运行该命令前，请先检查跳帽J26是否已接上。

测试时可使用./adc.sh脚本，运行时旋转电位器可看到采集的电压变化。

#在脚本目录下运行

.. code-block:: sh
   :linenos:

   ./adc.sh

.. image:: media/script006.jpg
   :align: center
   :alt: 未找到图片6|



陀螺仪
~~~~~~~~~~~~~

EBF6ULL Pro开发板带有陀螺仪，可进行陀螺仪测试。

测试时可使用./mpu_demo应用程序，运行时调整开发板的姿态可看到陀螺仪输出的加速度、角速度会发生变化。

#在脚本目录下运行

.. code-block:: sh
   :linenos:

   ./mpu_demo

.. image:: media/script007.png
   :align: center
   :alt: 未找到图片7|



与前面的sh脚本不同，mpu_demo是一个C程序生成的可执行文件，在后面的章节我们会进行源码讲解。

音频播放
~~~~~~~~~~~~~~~~~~~~~~~~

EBF6ULL Pro开发板带有耳机接口，可进行音乐播放测试。

测试前使用耳机连接至开发板。

使用./ music_player.sh脚本进行测试。

#在脚本目录下运行

.. code-block:: sh
   :linenos:

   ./music_player.sh

.. image:: media/script008.png
   :align: center
   :alt: 未找到图片8|



脚本执行时，可在耳机中听到播放的音乐。

视频播放
~~~~~~~~~~~~~~~~~~~~~~~~

EBF6ULL Pro开发板带有耳机及液晶接口，可进行视频播放测试。

测试前给开发板连接好耳机及液晶屏。

使用./ video_player.sh脚本可进行视频播放测试。

#在脚本目录下运行

.. code-block:: sh
   :linenos:

   ./video_player.sh

.. image:: media/script009.png
   :align: center
   :alt: 未找到图片9|



脚本执行时，可在屏幕看到播放的视频，耳机会有视频的声音。

其它
~~~~~~~~~~~~

EBF6ULL Pro开发板上还有DHT11、摄像头、WiFi等硬件接口，不过由于开
发板存在硬件引脚的复用，控制某些硬件时需要使用非出厂自带
的Linux系统设备树，控制这些硬件的方式我们将在学习切换系统设备树后再进行操作。






.. |script002| image:: media/script002.png
   :width: 4.45833in
   :height: 0.8896in
.. |script003| image:: media/script003.png
   :width: 3.52778in
   :height: 2.46293in
.. |script004| image:: media/script004.png
   :width: 5.125in
   :height: 2.89705in
.. |script005| image:: media/script005.jpg
   :width: 3.52273in
   :height: 0.42424in
.. |script006| image:: media/script006.jpg
   :width: 3.40833in
   :height: 1.58333in
.. |script007| image:: media/script007.png
   :width: 3.02905in
   :height: 2.19861in
.. |script008| image:: media/script008.png
   :width: 4.49333in
   :height: 4.68056in
.. |script009| image:: media/script009.png
   :width: 4.51721in
   :height: 4.69444in
