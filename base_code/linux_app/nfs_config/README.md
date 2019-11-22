# NFS文件系统配置说明


## 主机配置 

1. 安装NFS服务
Ubuntu系统默认没有安装NFS服务，需要使用如下命令安装NFS服务端软件：

```bash
sudo   apt-get   install  nfs-kernel-server
```

2. 查看用户和组id
先运行命令id查看主机的用户uid和gid，方便后面使用id进行配置
``` bash
embedfire@dev:~$ id
uid=998(embedfire) gid=998(embedfire) 组=998(embedfire)
```
本示例中embedfire的用户uid为998，组gid为998。

3. 修改 /etc/exports 文件的内容，
在文件末尾添加如下配置语句：

``` config
/home/embedfire/workdir 192.168.0.0/24(rw,sync,anonuid=998,anongid=998,no_subtree_check)

```

该配置说明如下：
- /home/embedfire/workdir：要用于NFS文件系统共享的主机目录
- 192.168.0.0/24：配置谁可以访问，此处配置IP为 192.168.0.* 的主机均可以访问该目录，即局域网上的所有主机，若局域网是其它网段，请参考此处的配置，不要用星号，如192.168.1.* 的局域网请配置为 192.168.1.0/24
- rw: 表示客户机的权限，r为可读，w为可写，具体的权限还受文件系统的rwx及用户身份影响
- sync：资料同步写入到内存与硬盘中
- anonuid=998：将客户机上的用户映射成指定的本地用户ID的用户，此处998是
- anongid=998： 将客户机上的用户映射成属于指定的本地用户组ID
- no_subtree_check：不检查子目录权限，默认配置，不写会提示警告


4. 共享需要确保目录存在。
创建共享目录可使用如下命令：

```bash
#创建共享目录，前面示例中共享的是/home/embedfire/workdir
mkdir /home/embedfire/workdir
```

5. 更新exports配置。

修改完/etc/exports文件并保存后，可使用exportfs命令更新配置：
```bash
sudo   exportfs   -arv
```

该命令的参数说明如下：
* -a：全部mount或umount文件/etc/exports中的内容。
* -r：重新mount文件/etc/exports中的共享内容。
* -u：umount目录。
* -v：在exportfs的时候，将详细的信息输出到屏幕上。


6.	查看NFS共享情况
使用showmount  –e 可查看当前NFS服务器的加载情况。
```bash
showmount   -e

```
---------

## 客户端配置

以下操作均在`客户端（开发板）的终端`上进行，默认用户为root。
以下操作均在`客户端（开发板）的终端`上进行，默认用户为root。
以下操作均在`客户端（开发板）的终端`上进行，默认用户为root。

1.	创建挂载点
    挂载时需要本地存在该目录，本例子把开发板挂载在/home/root/mountnfs目录下，首先需要创建该目录：
    ```bash
    #以下命令在开发板上运行
    mkdir    /home/root/mountnfs
    ```

2. 临时挂载NFS文件系统

    接下来使用mount命令进行挂载：
    ```bash
    #以下命令在开发板上运行
    #需要把下面的dev设置为前面自己的主机名
    mount  -o  vers=4   dev:/home/embedfire/workdir   /home/root/mountnfs
    ```

        以上命令使用的各个参数如下：
        * -o vers=4：表示使用NFS文件系统第4版本，若不注明版本可能会提示参数错误。
        * dev：目标主机的名字，此处也可以直接使用目标主机的IP地址，如本例子的目标机器IP为192.168.0.219。
        * /home/embedfire/workdir：远端的主机共享目录。
        * /home/root/mountnfs：本地挂载点，即要把远端共享的目录映射到本地的哪个目录。
    若挂载成功，终端不会有输出，Linux下执行命令后若没有输出通常就是表示执行成功。
    使用这种方式挂载目录只是临时的，开发板在重启后需要重新挂载该NFS目录才能访问。

3.	使用脚本进行测试
    挂载成功后，可以在开发机上把前面编写的hello world脚本复制至共享目录，然后在开发板上的终端直接访问该脚本文件并执行。在后面我们都会采用这样的形式进行开发，即在开发主机编写代码，复制至共享目录，然后在开发板上测试。

4.	取消挂载
    当客户机在网络上无法找到NFS共享的目录时，如开发主机关机时，在NFS的客户机的终端常常会输出一些提示，或在使用ls命令查看共享目录会导致长时间等待，这时可以对目录使用umount命令取消挂载，示例如下：
    ```bash
    #以下命令在开发板上运行
    umount  /home/root/mountnfs
    ```
    使用该命令时以要取消挂载的目录作为参数即可，没有输出表示执行正常。如果在当前挂载的目录进行umount操作，会提示“device is busy”。建议取消挂载时，先切换到家目录”~”，在进行umount操作。

