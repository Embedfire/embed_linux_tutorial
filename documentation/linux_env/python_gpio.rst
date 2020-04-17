Debian搭建Python控制GPIO开发环境
================================

说明
----

-  平台：野火imx6ull开发板
-  系统：Debian
-  目标Python版本：Python3.7

**注意：此处的操作是在野火imx6ull开发板上（运行的是Debian系统），而非虚拟机。**

在上一章的教程中，我们通过Python控制OLED显示了一些东西，但是如果我们想要通过python控制某个io口怎么办呢，其实很简单，就是一样的操作。

如果你能通过python控制OLED
屏幕了，那么接下来的应用层代码是非常简单的，写就对了。

如果你还没有安装Python的环境，请参考: `在Debian系统中部署Python3.7运行环境 <https://tutorial.linux.doc.embedfire.com/zh_CN/latest/linux_env/python37_env.html>`__

如果你还没下载并安装 ``Adafruit_Blinka、Adafruit_CircuitPython_SSD1306、Adafruit_Python_PlatformDetect`` 这些库，请参考: `Debian搭建Python控制OLED开发环境 <https://tutorial.linux.doc.embedfire.com/zh_CN/latest/linux_env/python_ssd1306.html>`__ 。

修改pin引脚
-----------

首先进入 ``Adafruit_Blinka/src/adafruit_blinka/microcontroller/nxp_imx6ull/`` 目录下，打开 ``pin.py`` 文件，它里面是定义了底层接口的引脚：

定义引脚的方式如下：

.. code:: bash

    Pin(([GPIO组号 - 1], [GPIO引脚号]))

比如RGB
LED的引脚分别为GPIO1_IO4、GPIO4_IO20、GPIO4_IO19，蜂鸣器的引脚是GPIO1_IO19，按键的引脚是GPIO5_IO1，那么他们的定义就如下面的代码所示：

.. code:: py

    R_LED_PIN = Pin((0, 4))    # GPIO1_IO4
    G_LED_PIN = Pin((3, 20))    # GPIO4_IO20
    B_LED_PIN = Pin((3, 19))    # GPIO4_IO19

    BEEP_PIN = Pin((0, 19))    # GPIO1_IO19

    BUTTON_PIN = Pin((4, 1))    # GPIO5_IO1

定义好引脚的gpio后，我们可以把它写到board模块中，然后就可以从board模块导入这些引脚，直接编辑 ``Adafruit_Blinka/src/adafruit_blinka/board/embedfire_imx6ull.py`` 文件即可，在里面添加以下代码：

.. code:: py

    R_LED = pin.R_LED_PIN
    G_LED = pin.G_LED_PIN
    B_LED = pin.B_LED_PIN

    BEEP = pin.BEEP_PIN

    BUTTON = pin.BUTTON_PIN

如此一来在使用的时候就可以这样使用：

.. code:: py

    from board import R_LED, G_LED, B_LED

    from board import BEEP

    from board import BUTTON

就可以导入对应的GPIO，非常方便

如果你想要修改其他GPIO的话，一样是这样子操作，定义底层的pin，然后直接导入使用即可。

**补充说明：野火已经提供控制蜂鸣器、RGB
LED灯以及按键的GPIO接口，所以大家不需要去修改**

安装库
------

如果你修改了GPIO的pin引脚，那么需要重新安装这些库，安装的步骤如下：

-  进入Adafruit_CircuitPython_SSD1306目录下，安装对应的库：

.. code:: bash

    cd Adafruit_CircuitPython_SSD1306 

    sudo python setup.py install

-  然后进入Adafruit_Blinka目录下安装对应的库：

.. code:: bash

    cd Adafruit_Blinka

    sudo python setup.py install

-  最后进入Adafruit_Python_PlatformDetect目录下安装对应的库：

.. code:: bash

    cd Adafruit_Python_PlatformDetect

    sudo python setup.py install

编写控制GPIO的代码
------------------

野火提供的GPIO应用代码有3个，分别控制RGB
LED、蜂鸣器、按键，核心的依赖库是digitalio，它里面提供了GPIO初始化的方法、控制GPIO方向（如配置为输入方向、配置为输出方向）、设置GPIO的电平、读取GPIO的电平等。

python控制RGB LED
~~~~~~~~~~~~~~~~~

控制RGB LED的代码可以从
https://github.com/Embedfire-pythonlib/Adafruit_CircuitPython_SSD1306/blob/master/examples/rgb_led.py
得到。

代码如下：

.. code:: py

    import time
    import digitalio
    from board import R_LED, G_LED, B_LED

    rled = digitalio.DigitalInOut(R_LED)
    gled = digitalio.DigitalInOut(G_LED)
    bled = digitalio.DigitalInOut(B_LED)

    rled.switch_to_output()     
    gled.switch_to_output()
    bled.switch_to_output()

    while True:
        rled.value = 0
        time.sleep(1)
        rled.value = 1
        gled.value = 0
        time.sleep(1)
        gled.value = 1
        bled.value = 0
        time.sleep(1)
        bled.value = 1
        time.sleep(1)

代码简单解析：

-  通过 ``digitalio.DigitalInOut`` 初始化一个pin对象

-  使用 ``switch_to_output`` 方法选择为输出方向，如果是输入方向则使用 ``switch_to_input`` 方法。

-  直接设置 ``value`` ，输出高低电平。

python控制蜂鸣器
~~~~~~~~~~~~~~~~

控制蜂鸣器的代码可以从
https://github.com/Embedfire-pythonlib/Adafruit_CircuitPython_SSD1306/blob/master/examples/beep_onoff.py
得到。

代码如下：

.. code:: py

    import time
    import digitalio
    from board import BEEP

    beep = digitalio.DigitalInOut(BEEP)
    beep.switch_to_output()

    while True:
        beep.value = 0
        time.sleep(1)
        beep.value = 1
        time.sleep(1)

python控制按键
~~~~~~~~~~~~~~

控制按键的代码可以从
https://github.com/Embedfire-pythonlib/Adafruit_CircuitPython_SSD1306/blob/master/examples/button.py
得到。

按键是作为输入的方向，在应用层读取gpio的电平，当按键被按下是，终端打印对应的信息。

代码如下：

.. code:: py

    import time
    import digitalio
    from board import BUTTON

    button = digitalio.DigitalInOut(BUTTON)
    button.switch_to_input()

    print("please touch the button!")

    while True:
        if (button.value):
            print("the button is pressed...")
            time.sleep(1)

实验现象
--------

代码的实验现象比较简单，直接运行测试即可，运行的方式如下（必须有python环境）:

.. code:: bash

    python [xxx.py]

