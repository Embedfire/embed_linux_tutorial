在Debian使用SQLite
==================

-  平台：野火imx6ull开发板
-  系统：Debian
-  SQLite版本：SQLite3

**注意：此处的操作是在野火imx6ull开发板上（运行的是Debian系统），而非虚拟机。**

SQLite 简介
-----------

SQLite是一款轻量级数据库，是一个关系型数据库（RDBMS）管理系统，它包含在一个相对小的C库中，实现了自给自足的、无服务器的、零配置的、事务性的
SQL
数据库引擎。在很多嵌入式产品中使用了它，它占用资源非常的低，在嵌入式设备中，可能只需要几百K的内存就够了。它能够支持Windows/Linux/Unix/Android/IOS等等主流的操作系统，同时能够跟很多程序语言相结合，比如
C#、PHP、Java等，更重要的是 SQLite
文件格式稳定，跨平台且向后兼容，开发人员保证至少在2050年之前保持这种格式。而且速度极快，目前是在世界上最广泛部署的
SQL 数据库，据官网统计活跃使用的SQLite数据库超过1万亿，更重要的是SQLite
源代码不受版权限制，任何人都可以免费使用于任何目的。

大家可以参考官网的介绍： https://www.sqlite.org/index.html

安装SQLite
----------

我们要在Debian开发板上安装SQLite，使用的版本是SQLite3。

SQLite
的一个重要的特性是零配置的，这意味着不需要复杂的安装或管理，直接一条命令安装即可。

为了确保我们的版本是最新版本，让我们使用apt命令更新本地apt包索引和升级系统：

.. code:: bash

    sudo apt-get update
    sudo apt-get -y upgrade

``-y`` 标志将确认我们同意所有要安装的项目。

.. code:: bash

    sudo apt-get -y install sqlite3

因为SQLite非常小，很快就安装完了，可以看到安装完成的最后输出了SQLite的版本信息，所以安装的版本是 ``3.27.2-3`` ：

.. code:: bash

    ···
    Preparing to unpack .../sqlite3_3.27.2-3_armhf.deb ...
    Unpacking sqlite3 (3.27.2-3) ...
    Setting up sqlite3 (3.27.2-3) ...

SQLite命令
----------

我们简单讲解 SQLite 编程所使用的简单却有用的命令，这些命令被称为 SQLite
的点命令。

在终端运行 ``sqlite3`` 命令，可以发现进入了 SQLite
命令终端，并且显示了SQLite版本相关的信息

在 SQLite 命令提示符下，我们可以使用各种 SQLite
的点命令，比如我们输入一个 ``.help`` 命令，这是一个帮助命令，可以帮助开发者查看
SQLite
支持所有的点命令，注意，这个help命令前有一个 **点** 的，这也是为什么这些命令被称之为点命令的原因。

当输入 ``.help`` 命令后，可以看到非常多的点命令：

.. code:: bash

    ➜  ~ sqlite3
    SQLite version 3.27.2 2019-02-25 16:06:06
    Enter ".help" for usage hints.
    Connected to a transient in-memory database.
    Use ".open FILE name" to reopen on a persistent database.
    sqlite> .help
    .archive ...             Manage SQL archives
    .auth ON|OFF             Show authorizer callbacks
    .backup ?DB? FILE        Backup DB (default "main") to FILE
    .bail on|off             Stop after hitting an error.  Default OFF
    .binary on|off           Turn binary output on or off.  Default OFF
    .cd DIRECTORY            Change the working directory to DIRECTORY
    .changes on|off          Show number of rows changed by SQL
    .check GLOB              Fail if output since .testcase does not match
    .clone NEWDB             Clone data into NEWDB from the existing database
    .databases               List names and files of attached databases
    .dbconfig ?op? ?val?     List or change sqlite3_db_config() options
    .dbinfo ?DB?             Show status information about the database
    .dump ?TABLE? ...        Render all database content as SQL
    .echo on|off             Turn command echo on or off
    ···
    ···

其实这里的命令非常多，我只截取了一部分，以下是部分点命令的功能介绍，其实没必要全部记住，在需要的时候通过 ``.help`` 命令查询一下即可：

+--------------------------+-------------------------------------------------------------------------------------------+
| 命令                     | 功能                                                                                      |
+==========================+===========================================================================================+
| .archive ...             | 管理SQL存档                                                                               |
+--------------------------+-------------------------------------------------------------------------------------------+
| .auth ON \| OFF          | 显示授权者回调                                                                            |
+--------------------------+-------------------------------------------------------------------------------------------+
| .backup ?DB? FILE        | 备份 DB 数据库（默认是 "main"）到 FILE 文件                                               |
+--------------------------+-------------------------------------------------------------------------------------------+
| .restore ?DB? FILE       | 从FILE文件恢复DB的内容（默认为“ main”）                                                   |
+--------------------------+-------------------------------------------------------------------------------------------+
| .databases               | 列出数据库的名称及其所依附的文件                                                          |
+--------------------------+-------------------------------------------------------------------------------------------+
| .dump ?TABLE?            | 以 SQL 文本格式转储数据库。如果指定了 TABLE 表，则只转储匹配 LIKE 模式的 TABLE 表。       |
+--------------------------+-------------------------------------------------------------------------------------------+
| .echo ON\|OFF            | 开启或关闭 echo 命令。                                                                    |
+--------------------------+-------------------------------------------------------------------------------------------+
| .import FILE TABLE       | 导入来自 FILE 文件的数据到 TABLE 表中。                                                   |
+--------------------------+-------------------------------------------------------------------------------------------+
| .indices ?TABLE?         | 显示所有索引的名称。如果指定了 TABLE 表，则只显示匹配 LIKE 模式的 TABLE 表的索引。        |
+--------------------------+-------------------------------------------------------------------------------------------+
| .log FILE\|off           | 开启或关闭日志。FILE 文件可以是 stderr（标准错误）/stdout（标准输出）。                   |
+--------------------------+-------------------------------------------------------------------------------------------+
| .mode MODE ?TABLE?       | 设置输出模式，可选泽的模式：csv 、column 、html、insert、line 、list、list 、tabs 、tcl   |
+--------------------------+-------------------------------------------------------------------------------------------+
| .open ?OPTIONS? ?FILE?   | 关闭现有的数据库并重新打开文件                                                            |
+--------------------------+-------------------------------------------------------------------------------------------+
| .output FILE name        | 发送输出到 FILE name 文件。                                                               |
+--------------------------+-------------------------------------------------------------------------------------------+
| .output stdout           | 发送输出到屏幕。                                                                          |
+--------------------------+-------------------------------------------------------------------------------------------+
| .print STRING...         | 逐字地输出 STRING 字符串。                                                                |
+--------------------------+-------------------------------------------------------------------------------------------+
| .quit                    | 退出 SQLite 提示符终端。                                                                  |
+--------------------------+-------------------------------------------------------------------------------------------+
| .read FILE name          | 从FILE读取输入                                                                            |
+--------------------------+-------------------------------------------------------------------------------------------+
| .schema ?PATTERN?        | 显示与PATTERN匹配的CREATE语句                                                             |
+--------------------------+-------------------------------------------------------------------------------------------+
| .show                    | 显示各种设置的当前值                                                                      |
+--------------------------+-------------------------------------------------------------------------------------------+
| .stats ON\|OFF           | 开启或关闭统计。                                                                          |
+--------------------------+-------------------------------------------------------------------------------------------+
| .tables ?TABLE?          | 列出与LIKE模式TABLE匹配的表的名称                                                         |
+--------------------------+-------------------------------------------------------------------------------------------+

创建数据库
----------

首先我们创建一个数据库，SQLite 的 sqlite3 命令被用来创建新的 SQLite
数据库，我们不需要任何特殊的权限即可创建一个数据，如果路径下存在相同名字的数据库则直接打开该数据库而不是重新创建。

-  创建test.db数据库

.. code:: bash

    ➜  ~ sqlite3 test.db  

    SQLite version 3.27.2 2019-02-25 16:06:06
    Enter ".help" for usage hints.
    sqlite>

创建表
------

SQLite 的 ``CREATE TABLE``
语句用于在任何给定的数据库创建一个新表。创建基本表，涉及到命名表、定义列及每一列的数据类型。

``CREATE TABLE`` 语句的基本语法如下：

.. code:: bash

    CREATE TABLE database_name.table_name(
       column1 datatype  PRIMARY KEY(one or more columns),
       column2 datatype,
       column3 datatype,
       .....
       columnN datatype,
    );

-  ``database_name.table_name`` 在数据库中是唯一的。
-  ``column1, column2, ... columnN`` ：表示的是数据库中的列。
-  ``datatype`` ：每列的数据类型，可选INT、TEXT、CHAR、REAL等类型。
-  ``PRIMARY`` ：表示第1列作为主键。
-  ``KEY`` ：关键字，一个或者多个。

比如下面的语句就创建一个班级，班级中有学生的id，名字、年龄、总分数等列，id
作为主键， ``NOT NULL`` 的约束表示在表中创建纪录时这些字段不能为
NULL，此处使用小写字母表示用户可以修改的内容。

.. code:: bash

    CREATE TABLE class(
       id INT PRIMARY KEY     NOT NULL,
       name           TEXT    NOT NULL,
       age            INT     NOT NULL,
       score          INT
    );

这样子创建的数据库就类似一个Excel表格，差不多是这样子的类型：

.. code:: bash

    id          name        age         score     
    ----------  ----------  ----------  ----------

在创建完成后，可以使用 SQLIte 命令中的 ``.tables``
命令来验证表是否已成功创建，该命令用于列出附加数据库中的所有表。

.. code:: bash

    sqlite> .tables
    class

插入数据
--------

SQLite 的 ``INSERT INTO`` 语句用于向数据库的某个表中添加新的数据行。

``INSERT INTO`` 语句有两种基本语法。

为表中部分列添加对应的值，可以通过以下方法：

.. code:: bash

    INSERT INTO TABLE_NAME (column1, column2, column3,...columnN)  
    VALUES (value1, value2, value3,...valueN);

-  ``TABLE_NAME`` ：表示表的名字，是数据库中唯一的。

-  ``column1, column2, ... columnN`` ：表示要插入数据的表中的列的 **名称** ，注意这里是列的名称。

-  ``value1, value2, value3, ... valueN`` ：表示要插入数据的表中的列的值。

如果要为表中的所有列添加值，我们也可以不需要在 SQLite
查询中指定列名称，但要确保值的顺序与列在表中的顺序一致，SQLite 的 INSERT
INTO 语法如下：

.. code:: bash

    INSERT INTO TABLE_NAME VALUES (value1,value2,value3,...valueN);

-  TABLE_NAME：表示表的名字，是数据库中唯一的。

-  value1, value2, value3,...valueN：表示要插入数据的表中的列的值。

**注意，插入值的类型要与表中列指定的类型要匹配的。**

我们来给class表添加对应的值：

-  语法1格式：

.. code:: bash

    INSERT INTO class (id, name, age, score)
    VALUES (1, 'liuyi', 22, 610);

    INSERT INTO class (id, name, age, score)
    VALUES (2, 'chener', 19, 621);

    INSERT INTO class (id, name, age, score)
    VALUES (3, 'zhangsan', 23, 601);

    INSERT INTO class (id, name, age, score)
    VALUES (4, 'lisi', 21, 666);

    INSERT INTO class (id, name, age, score)
    VALUES (5, 'wangwu', 20, 629);

    INSERT INTO class (id, name, age, score)
    VALUES (6, 'zhaoliu', 22, 621);

    INSERT INTO class (id, name, age, score)
    VALUES (7, 'sunqi', 20, 611);

    INSERT INTO class (id, name, age, score)
    VALUES (8, 'zhouba', 22, 591);

    INSERT INTO class (id, name, age, score)
    VALUES (9, 'wujiu', 23, 625);

    INSERT INTO class (id, name, age, score)
    VALUES (10, 'zhengshi', 21, 621);

-  语法2格式：

.. code:: bash

    INSERT INTO class VALUES (1, 'liuyi', 22, 610);

    INSERT INTO class VALUES (2, 'chener', 19, 621);

    INSERT INTO class VALUES (3, 'zhangsan', 23, 601);

    INSERT INTO class VALUES (4, 'lisi', 21, 666);

    INSERT INTO class VALUES (5, 'wangwu', 20, 629);

    INSERT INTO class VALUES (6, 'zhaoliu', 22, 621);

    INSERT INTO class VALUES (7, 'sunqi', 20, 611);

    INSERT INTO class VALUES (8, 'zhouba', 22, 591);

    INSERT INTO class VALUES (9, 'wujiu', 23, 625);

    INSERT INTO class VALUES (10, 'zhengshi', 21, 621);

这两个语法得出的结果是一样的，随便选择一个即可。

查找数据
--------

SQLite 的 ``SELECT`` 语句用于从 SQLite
数据库表中获取数据，并且以结果表的形式返回数据。

SQLite 的 ``SELECT`` 语句的基本语法如下：

.. code:: bash

    SELECT column1, column2, ... columnN FROM table_name;

-  ``column1, column2, ... columnN`` ：表示要查找数据的表中的列的 **名称** ，注意这里是列的名称，当然可以使用
   ``*`` 符号表示要查找所有的列。

-  ``table_name`` ：表示要查找数据库中的表名称，它在数据库中是唯一的。

查找刚刚创建的class表操作如下：

.. code:: bash

    sqlite> SELECT * FROM class;

    1|liuyi|22|610
    2|chener|19|621
    3|zhangsan|23|601
    4|lisi|21|666
    5|wangwu|20|629
    6|zhaoliu|22|621
    7|sunqi|20|611
    8|zhouba|22|591
    9|wujiu|23|625
    10|zhengshi|21|621

你会发现这些表的格式是很乱，不直观，那么你可以通过 ``.header`` 命令显示表头，通过 ``.mode`` 设置显示的模式：

.. code:: bash

    .header on
    .mode column

然后再次查找所有的内容：

.. code:: bash

    sqlite> SELECT * FROM class;

    id          name        age         score     
    ----------  ----------  ----------  ----------
    1           liuyi       22          610       
    2           chener      19          621       
    3           zhangsan    23          601       
    4           lisi        21          666       
    5           wangwu      20          629       
    6           zhaoliu     22          621       
    7           sunqi       20          611       
    8           zhouba      22          591       
    9           wujiu       23          625       
    10          zhengshi    21          621   

这一次就好看多了，当然你还可以进行排序。

数据排序
--------

SQLite 的 ``ORDER BY``
语句是用来基于一个或多个列按升序或降序顺序排列数据。

``ORDER BY`` 语句的基本语法如下：

.. code:: bash

    SELECT column-list 
    FROM table_name 
    [WHERE condition] 
    [ORDER BY column1, column2, .. columnN] [ASC | DESC];

比如按照某一列进行排序，此处选择 ``score`` 列，ASC表示升序，DESC表示降序：

.. code:: bash

    sqlite> SELECT * FROM class ORDER BY score ASC;

    id          name        age         score     
    ----------  ----------  ----------  ----------
    8           zhouba      22          591       
    3           zhangsan    23          601       
    1           liuyi       22          610       
    7           sunqi       20          611       
    2           chener      19          621       
    6           zhaoliu     22          621       
    10          zhengshi    21          621       
    9           wujiu       23          625       
    5           wangwu      20          629       
    4           lisi        21          666 

.. code:: bash

    sqlite> SELECT * FROM class ORDER BY score DESC;

    id          name        age         score     
    ----------  ----------  ----------  ----------
    4           lisi        21          666       
    5           wangwu      20          629       
    9           wujiu       23          625       
    2           chener      19          621       
    6           zhaoliu     22          621       
    10          zhengshi    21          621       
    7           sunqi       20          611       
    1           liuyi       22          610       
    3           zhangsan    23          601       
    8           zhouba      22          591   

导出数据库为SQL脚本
-------------------

我们可以通过 ``.dump`` 命令导出数据库为一个sql脚本，也可以通过这个脚本还原一个数据库，具体操作如下：

在shell终端中（注意不是在sqlite提示符终端）通过以下命令即可导出数据库为sql脚本：

.. code:: bash

    sqlite3 test.db .dump > test.sql 

.. code:: bash

    # 当前路径下存在test.db数据库
    ➜  ~ ls
    bin  mountnfs  qt-app  test.db

    # 导出数据库为sql脚本
    ➜  ~ sqlite3 test.db .dump > test.sql 

    # 生成test.sql脚本
    ➜  ~ ls
    bin  mountnfs  qt-app  test.db  test.sql

    # 查看脚本的内容
    ➜  ~ cat test.sql 
    PRAGMA foreign_keys=OFF;
    BEGIN TRANSACTION;
    CREATE TABLE class(
       id INT PRIMARY KEY     NOT NULL,
       name           TEXT    NOT NULL,
       age            INT     NOT NULL,
       score          INT
    );
    INSERT INTO class VALUES(1,'liuyi',22,610);
    INSERT INTO class VALUES(2,'chener',19,621);
    INSERT INTO class VALUES(3,'zhangsan',23,601);
    INSERT INTO class VALUES(4,'lisi',21,666);
    INSERT INTO class VALUES(5,'wangwu',20,629);
    INSERT INTO class VALUES(6,'zhaoliu',22,621);
    INSERT INTO class VALUES(7,'sunqi',20,611);
    INSERT INTO class VALUES(8,'zhouba',22,591);
    INSERT INTO class VALUES(9,'wujiu',23,625);
    INSERT INTO class VALUES(10,'zhengshi',21,621);
    COMMIT;

从SQL脚本导入数据库
-------------------

这一步的操作与上一步操作是相反的，可以通过以下命令导入SQL脚本生成一个新的数据库：

.. code:: bash

    sqlite3 new.db < test.sql

注意，这里的new.db
是通过test.sql生成的，它里面的内容与test.db是完全一致的。

.. code:: bash

    # 从SQL脚本导入数据库
    ➜  ~ sqlite3 new.db < test.sql

    # 生成新的数据库new.db
    ➜  ~ ls
    bin  mountnfs  new.db  qt-app  test.db  test.sql

    # 进入新的数据库中并查询数据表
    ➜  ~ sqlite3 new.db 
    SQLite version 3.27.2 2019-02-25 16:06:06
    Enter ".help" for usage hints.
    sqlite> .header on
    sqlite> .mode column
    sqlite> SELECT * FROM class;
    id          name        age         score     
    ----------  ----------  ----------  ----------
    1           liuyi       22          610       
    2           chener      19          621       
    3           zhangsan    23          601       
    4           lisi        21          666       
    5           wangwu      20          629       
    6           zhaoliu     22          621       
    7           sunqi       20          611       
    8           zhouba      22          591       
    9           wujiu       23          625       
    10          zhengshi    21          621     

至此，数据库的讲解完成，更多知识大家可以自行学习，此处仅告诉大家野火开发板上可以使用数据库。
