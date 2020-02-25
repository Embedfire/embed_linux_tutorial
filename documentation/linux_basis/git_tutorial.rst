.. vim: syntax=rst

Git教程
------------------

Git是一个功能十分强大的分布式版本控制系统，主要用于有效、高速地对各种各样的项目进行版本管理。

Git的诞生，起源于linux社区的一位大牛Andrew(samba之父)写了可连接BitKeeper仓库的外挂，
这引起了BitMover 公司的震怒，收回了对linux社区免费使用BitKeeper的授权。于是Linux之父，
Torvalds，花了十天时间为Linux社区开发了新的版本工具--Git。

Git的特点
~~~~~~~~~~~~~~~~~~

相较于其他版本管理工具，Git有以下几个优点:

- 快照记录。git和其他版本管理工具重要差别在于存储数据的方式。
  其他版本工具通常将它们保存的信息看作是一种原始数据+文件修改的记录。
  而Git在储存时是生成快照来记录全部文件（快照不是所有文件的单纯拷贝，而是一种对全体文件的索引）。

- 分布式。每一位项目开发人员的电脑里，都有项目的完成备份，避免造成资料受损无法挽回的后果。
  同时，本地的项目备份使得所有的版本管理操作都在毫秒级别内完成。
  这与SVN、CVS等集中式版本工具时常要等待中央服务器响应相比,用户体验显得非常好。

- 开源。这与linux内核一样，开放源码保证了Git的可靠性和安全性，也有利于Git的功能越来越强大。


Git的学习层次
~~~~~~~~~~~~~~~~~~~~~~~~~~

每一个Git的学习者，在遇上Git复杂繁多的命令时，都难免心里发怵。客观来说，Git确实不及SVN那么简单好用，
新手要用Git好不容易。但是换个角度看，若是我们能熟练掌握Git，体验到其中的奥妙，
会发现自己再也不愿意回到SVN的怀抱了。

王国维在<<人间词话>>里说:欢古今之成大事业、大学问者，必经过三种之境界:"昨夜西风凋碧树。
独上高楼，望尽天涯路。"此第一境也。"衣带渐宽终不悔，为伊消得人憔悴。"此第二镜也。
"众里寻他千百度，那人却在灯火阑珊处。"此第三镜也。

Git的学习同样也可以分为三个层次，用户可以根据自己实际情况进行学习。

- 第一层，Git的基础使用，这一阶段适合小白用户学习，主要是学习利用Git来下载一些开源项目，学会对项目进行增删改查。

- 第二层，Git的分支、标签、版本管理等相关用法，这一阶段适合在工作中频繁使用Git来进行开发的用户，学会进行版本追溯和各种冲突解决。

- 第三次，Git的版本库分析，这一阶段适合对Git原理感兴趣的开发者学习，深入了解Git在命令行的背后，对版本库进行了什么样的操作。


Git的安装
~~~~~~~~~~~~~~~~~

Windows系统

到Git官网下载安装包,直接安装即可。

https://gitforwindows.org


Linux系统

根据不同发行版，使用相关包管理工具安装即可。

比如Ubantu：在终端执行"sudo apt-get install git -y"即可。


Git的基础使用
~~~~~~~~~~~~~~~~~~~~

Git里面有很多比较晦涩的专业术语，如果不熟悉这一套术语，常常会在Git出现各种提示信息的时候摸不着头脑。
这里中文翻译了Git中比较重要的部分术语。先对这些专业术语有初步的印象，后面的教程再逐步讲解它的含义。

.. csv-table:: Frozen Delights!
    :widths: 10, 10, 10 ,10 ,10 ,10

    "respository","仓库/版本库","commit","提交","snapshot","快照"
    "SSH","安全传输协议","fetch","获取代码","SHA1","哈希值"
    "HEAD","当前分支","checkout","检出分支","Gerrit","代码审核"
    "pull","拉取代码","rebase","分支衍合","stash","储存" 
    "push","推送代码","tag","标签","index","索引" 
    "merge","分支合并","master","主分支","cherry-pick","条件分支"
    "add","添加改动","origin","远程分支","revert","反转提交"


Git本地仓库操作
^^^^^^^^^^^^^^^^^^^^^

1、Git安装完成后，新建一个文件夹"Git_demo"。右键点击，选择"Git Bash Here"选项打开,
输入"git init"命令。该指令会在当前目录下生成一个隐藏文件".git"，这个".git"文件就是术语中本地仓库/版本库。
如下图

.. image:: media/git_init.png
   :align: center
   :alt: git_init

2、添加一个newadd.txt文件，使用"git status"命令查看状态。

.. image:: media/git_status1.png
   :align: center
   :alt: git_status

此时新添加的文件处于"Untracked"未跟踪状态，提示使用"git add"命令添加文件到暂存区。

3、执行"git add newadd.txt"命令，重新查看状态。

.. image:: media/git_add.png
   :align: center
   :alt: git_add

 提示:new file：newadd.txt，说明文件已经添加到了暂存区。

 4、输入"git commit -m "add a new",把暂存区新修改的文件提交到仓库里面去保存，
并在仓库中用"add a new"这条语句来记录这一次的提交。

.. image:: media/git_commit.png
   :align: center
   :alt: git_commit

5、查看当前状态，git提示没有新的可提交文件，说明文件已经提交到仓库中去了。

.. image:: media/git_status2.png
   :align: center
   :alt: git_status

6、在newadd.txt文件中加入一句"change test",保存退出后，查看状态。

.. image:: media/git_status3.png
   :align: center
   :alt: git_status

此时git不再提示这是一个新文件，而是一个已经修改(modified)的文件。

7、重复前面提到添加到暂存区、提交到仓库的步骤，那这个改动的文件就已经保存下来了。
使用"git log"命令可以查看到这两次提交的记录。如下图:

.. image:: media/git_log.png
   :align: center
   :alt: git_log

在输出的历史记录中，commit后面跟着的一串数字，是该文件的SHA-1 校验和，
校验和的前八位称为commit-id。"git show"命令可以通过commit-id，
找到文件对应的修改记录。

8、执行"git show a96f85f1"命令，可以看到非常详细的修改记录，包括作者、
时间以及详细的修改内容。+号就代表新加的内容，-号代表删减的内容。

.. image:: media/git_show.png
   :align: center
   :alt: git_show

9、如果是特别关注某个文件，还有一个"git blame"命令可以迅速地找到这个文件的改动。
输入"git blame newadd.txt",该文件每次被提交仓库的commit-id、作者、时间、内容都显示出来了。
如下图:

.. image:: media/git_blame.png
   :align: center
   :alt: git_blame

持续补充中...