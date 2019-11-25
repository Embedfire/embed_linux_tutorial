# Readme.md
## 文件夹说明
### user<br>
+ main.c：主函数
### src<br>
+ bsp_led.c：LED灯相关的函数定义：<br>     
    + <span>LED_GPIO_Config</span>：用于初始化LED的GPIO引脚。GPIO4对应的是红灯；GPIO116对应的是绿灯；GPIO115对应的是蓝灯<br>
    + <span>LED_GPIO_DeInit</span>：复位LED灯相应的GPIO端口
### inc<br>
+ bsp_led.h：LED灯相关的函数声明，以及宏定义<br>
### objs<br>
+ 在编译过程中生成，用来存放编译过程中生成的目标文件（*.o）<br>
### Makefile
+ 本例中使用的Makefile文件
### ***run.sh***
+ 脚本文件，用来编译该工程
### ***led_demo***
+ 编译生成的可执行文件，可以直接在板子上运行。



## 使用方法
1. 编译工程<br>
    + **先切换到当前工程Makefile所在的目录**，再运行脚本run.sh，具体代码如下：<br>
    <span>./run.sh</span>
2. 执行程序<br>
    + 通过以下命令，将可执行文件led_demo放到共享文件夹中（此处使用的是nfs共享文件的方式，其他方式请百度）<br>
    <span>#***代表共享目录的路径，例如，我配置的是家目录的workdir文件夹，则应该是~/workdir</span><br>
    <span>cp led_demo ***</span>
    + 确保板子已经挂载上了nfs文件系统，切换到挂载的目录，执行以下命令<br>
    <span>./led_demo</span><br>
3. 若操作无误的话，我们可以看到板子的LED灯，轮流显示 “红绿蓝黄紫青白” 的颜色
---
# 补充说明
若想要执行Clean Project的操作，可以使用以下命令：<span>./run.sh clc</span>