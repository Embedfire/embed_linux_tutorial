在Debian系统中部署Python3.7运行环境
===================================

-  平台：野火imx6ull开发板
-  系统：Debian
-  目标Python版本：Python3.7

    注意：此处的操作是在野火imx6ull开发板上（运行的是Debian系统），而非虚拟机。

介绍
----

Python是一种灵活的多功能编程语言，适用于许多用例，包括脚本，自动化，数据分析，机器学习和后端开发。
1991年首次出版，其名称灵感来自英国喜剧组织Monty Python，开发团队希望让Python成为一种有趣的语言。
快速设置即时反馈错误，Python是一个有用的语言，可供初学者和经验丰富的开发人员学习。
Python3是该语言的最新版本，而Python2则逐渐被停止维护，并且Python3与Python2之间的差别还是很大的，
Python2的程序不一定能在Python3上运行，因此我们只使用Python3即可。

本教程将在Debian系统中搭建Python3开发环境，并且简单使用。

更新
----

其实在很多Debian系统中都预装了Python3或者Python2，但是野火提供的镜像是并未预装Python3的，
我们可以手动安装，为了确保我们的版本是最新版本，让我们使用apt命令更新本地apt包索引和升级系统：

.. code:: bash

    sudo apt-get update
    sudo apt-get -y upgrade

-y 标志将确认我们同意所有要安装的项目。

手动安装Python3
---------------

.. code:: bash

    sudo apt-get -y install python3

可以看到Python3有很多依赖的deb包：

.. code:: bash

    Reading package lists... Done
    Building dependency tree
    Reading state information... Done
    The following additional packages will be installed:
      libmpdec2 libpython3-stdlib libpython3.7-minimal libpython3.7-stdlib
      python3-minimal python3.7 python3.7-minimal
    Suggested packages:
      python3-doc python3-tk python3-venv python3.7-venv python3.7-doc binutils
      binfmt-support
    The following NEW packages will be installed:
      libmpdec2 libpython3-stdlib libpython3.7-minimal libpython3.7-stdlib python3
      python3-minimal python3.7 python3.7-minimal
    0 upgraded, 8 newly installed, 0 to remove and 0 not upgraded.
    Need to get 4224 kB of archives.
    After this operation, 19.6 MB of additional disk space will be used.

当然啦，这些依赖的deb包是不需要我们手动安装的，Debian会自动安装。

在安装完成后，校验是否安装成功：

.. code:: bash

     python3 -V

当出现以下内容时表示Python3安装成功：

.. code:: bash

    Python 3.7.3

设置Debian系统的默认Python版本
------------------------------

如果你是运行以下命令校验Python的版本：

.. code:: bash

    python -V

并且当你的系统存在Python2的话，那么版本可能就是 ``Python 2.7.16`` ，那么与我们想要安装的Python3版本并不符合，因此我们要设置Python3位默认版本：

.. code:: bash

    cd /usr/bin
    sudo rm python
    sudo ln -s python3.7 python
    cd ~

再次校验Python版本时，发现系统默认的Python版本就是python3.7了：

.. code:: bash

    python3 -V

    # 版本
    Python 3.7.3

简单使用Python
--------------

我们可以直接运行python命令进入Python的交互式编程，交互式编程不需要创建脚本文件，是通过
Python
解释器的交互模式进来编写代码。在这里可以做一些简单的操作，比如在中断打印"hello
world!"，做一些算术运算等，最后通过 ``exit()`` 退出：

.. code:: bash

    debian@npi:~$ python
    Python 3.7.3 (default, Dec 20 2019, 18:57:59)
    [GCC 8.3.0] on linux
    Type "help", "copyright", "credits" or "license" for more information.
    >>> 
    >>> print("hello world!")
    hello world!
    >>>
    >>> 8+2
    10
    >>> exit()

我们也可以编写一个简单的Python代码，保存为 ``hello.py`` ，注意要是.py格式的文件。

代码如下：

.. code:: py

    print("Hello, Python!")

然后运行：

.. code:: bash

    # 编写hello.py
    debian@npi:~$ ls
    hello.py 

    # 运行
    debian@npi:~$ python hello.py
    Hello, Python!

当然也可以运行一些比较复杂的Python代码，此处示例是获取某地（比如北京）的天气：

我们新建一个 ``weather.py`` ，写入以下代码：

.. code:: py

    # 导入json、requests包
    import json, requests

    #输入地点
    weather_place = "东莞"

    #日期
    date = []
    #最高温与最低温
    high_temp = []
    low_temp = []
    #天气
    weather = []

    # 请求天气信息
    weather_url = "http://wthrcdn.etouch.cn/weather_mini?city=%s" % (weather_place)

    response = requests.get(weather_url)
    try:
        response.raise_for_status()
    except:
        print("请求信息出错")
        
    #将json文件格式导入成python的格式
    weather_data = json.loads(response.text)

    # 打印原始数据
    # print(weather_data)
     
    w = weather_data['data']

    print("地点：%s" % w['city'])

    #进行五天的天气遍历，并格式化输出
    for i in range(len(w['forecast'])):
        date.append(w['forecast'][i]['date'])
        high_temp.append(w['forecast'][i]['high'])
        low_temp.append(w['forecast'][i]['low'])
        weather.append(w['forecast'][i]['type'])
        
        #输出
        print("日期：" + date[i])
        print("\t温度：最" + low_temp[i] + "\t最" + high_temp[i])
        print("\t天气：" + weather[i] + "\n")
        
    print("\n今日着装：" + w['ganmao'])
    print("当前温度：" + w['wendu'])

然后尝试运行：

.. code:: bash

    python weather.py

    # 输出
    Traceback (most recent call last):
      File "weather.py", line 1, in <module>
        import json, requests
    ModuleNotFoundError: No module named 'requests'

它会提示说没有 ``requests`` 模块（一般是被称之为包），那么怎么办呢，我们需要安装对应的包，才可以在python程序中正常使用。

安装pip包管理工具
-----------------

接下来我们就讲解怎么去安装对应的包。

pip是一个Python的包管理系统，允许用户安装Python包。使用pip，您可以从Python包索引（PyPI）和其他存储库安装对应的包。

我们首先还是更新一下apt的索引：

.. code:: bash

    sudo apt-get update

然后安装python3-pip：

.. code:: bash

    sudo apt-get -y install python3-pip

在安装完成后验证一下：

.. code:: bash

    pip3 --version

    # 出现以下内容表示安装成功
    pip 18.1 from /usr/lib/python3/dist-packages/pip (python 3.7)

使用pip包管理工具
-----------------

接下来我们就讲解怎么使用pip包管理工具去安装对应的包。用户可以从PyPI，版本控制，本地项目和分发文件安装软件包，但在大多数情况下，我们一般都是从PyPI安装软件包。

以为从上面的示例报错中发现，我是缺少一个名为 ``requests`` 的软件包，那么我们就可以通过以下命令去安装 ``requests`` 软件包：

.. code:: bash

    pip3 install requests

    # 输出内容：

    Collecting requests
    Downloading https://files.pythonhosted.org/packages/1a/70/1935c770cb3be6e3a8b78ced23d7e0f3b187f5cbfab4749523ed65d7c9b1/requests-2.23.0-py2.py3-none-any.whl (58kB)
        100% |████████████████████████████████| 61kB 16kB/s
    Collecting chardet<4,>=3.0.2 (from requests)
    Downloading https://files.pythonhosted.org/packages/bc/a9/01ffebfb562e4274b6487b4bb1ddec7ca55ec7510b22e4c51f14098443b8/chardet-3.0.4-py2.py3-none-any.whl (133kB)
        100% |████████████████████████████████| 143kB 16kB/s
    Collecting certifi>=2017.4.17 (from requests)
    Downloading https://files.pythonhosted.org/packages/b9/63/df50cac98ea0d5b006c55a399c3bf1db9da7b5a24de7890bc9cfd5dd9e99/certifi-2019.11.28-py2.py3-none-any.whl (156kB)
        100% |████████████████████████████████| 163kB 11kB/s
    Collecting urllib3!=1.25.0,!=1.25.1,<1.26,>=1.21.1 (from requests)
    Downloading https://files.pythonhosted.org/packages/e8/74/6e4f91745020f967d09332bb2b8b9b10090957334692eb88ea4afe91b77f/urllib3-1.25.8-py2.py3-none-any.whl (125kB)
        100% |████████████████████████████████| 133kB 11kB/s
    Collecting idna<3,>=2.5 (from requests)
    Downloading https://files.pythonhosted.org/packages/89/e3/afebe61c546d18fb1709a61bee788254b40e736cff7271c7de5de2dc4128/idna-2.9-py2.py3-none-any.whl (58kB)
        100% |████████████████████████████████| 61kB 12kB/s
    Installing collected packages: chardet, certifi, urllib3, idna, requests
    The script chardetect is installed in '/home/debian/.local/bin' which is not on PATH.
    Consider adding this directory to PATH or, if you prefer to suppress this warning, use --no-warn-script-location.
    Successfully installed certifi-2019.11.28 chardet-3.0.4 idna-2.9 requests-2.23.0 urllib3-1.25.8


然后重新运行weather.py程序

.. code:: bash

    python weather.py

    # 输出
    地点：东莞
    日期：27日星期五
            温度：最低温 21℃        最高温 28℃
            天气：阵雨

    日期：28日星期六
            温度：最低温 18℃        最高温 23℃
            天气：中到大雨

    日期：29日星期天
            温度：最低温 17℃        最高温 24℃
            天气：阴

    日期：30日星期一
            温度：最低温 21℃        最高温 26℃
            天气：阴

    日期：31日星期二
            温度：最低温 20℃        最高温 28℃
            天气：阵雨


    今日着装：天气转凉，空气湿度较大，较易发生感冒，体质较弱的朋友请注意适当防护。
    当前温度：27

pip的其他操作
-------------

卸载包：

.. code:: bash

    pip3 uninstall "package_name"

从PyPI搜索包：

.. code:: bash

    pip3 search "package_name"

列出已安装的包：

.. code:: bash

    pip3 list

列出过时的包：

.. code:: bash

    pip3 list --outdated

至此，Python3的环境搭建基本完成，更多内容大家可以自行探索。
