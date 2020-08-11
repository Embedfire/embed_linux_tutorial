.. vim: syntax=rst

添加LCD驱动
-------

如果使用我们提供的系统，默认情况下开启了LCD驱动。LCD跑起来还需要添加相应的设备节点。如何添加设备树插件以及如何修改LCD常用配置参数。

设备树插件源文件位置“~/embed_linux_tutorial/base_code/add_lcd_device_tree”。



LCD常用参数介绍
~~~~~~~~~

LCD配置参数主要包括LCD引脚相关配置和LCD显示参数配置。其中背光控制引脚是独立出来的，修改背光引脚的同时也要修改对应的PWM设备信息。

LCD引脚
^^^^^

设备树中引脚修改。

.. code-block:: c 
    :caption: lcd引脚
    :linenos:

    pinctrl_lcdif_ctrl: lcdifctrlgrp {
    	fsl,pins = <
    	    MX6UL_PAD_LCD_CLK__LCDIF_CLK	    0x79
    	    MX6UL_PAD_LCD_ENABLE__LCDIF_ENABLE  0x79
    	    MX6UL_PAD_LCD_HSYNC__LCDIF_HSYNC    0x79
    	    MX6UL_PAD_LCD_VSYNC__LCDIF_VSYNC    0x79
    	>;
    };
    pinctrl_lcdif_dat: lcdifdatgrp {
        fsl,pins = <
            MX6UL_PAD_LCD_DATA00__LCDIF_DATA00  0x79
            MX6UL_PAD_LCD_DATA01__LCDIF_DATA01  0x79
            MX6UL_PAD_LCD_DATA02__LCDIF_DATA02  0x79
            MX6UL_PAD_LCD_DATA03__LCDIF_DATA03  0x79
            MX6UL_PAD_LCD_DATA04__LCDIF_DATA04  0x79
            MX6UL_PAD_LCD_DATA05__LCDIF_DATA05  0x79
            MX6UL_PAD_LCD_DATA06__LCDIF_DATA06  0x79
            MX6UL_PAD_LCD_DATA07__LCDIF_DATA07  0x79
            MX6UL_PAD_LCD_DATA08__LCDIF_DATA08  0x79
            MX6UL_PAD_LCD_DATA09__LCDIF_DATA09  0x79
            MX6UL_PAD_LCD_DATA10__LCDIF_DATA10  0x79
            MX6UL_PAD_LCD_DATA11__LCDIF_DATA11  0x79
            MX6UL_PAD_LCD_DATA12__LCDIF_DATA12  0x79
            MX6UL_PAD_LCD_DATA13__LCDIF_DATA13  0x79
            MX6UL_PAD_LCD_DATA14__LCDIF_DATA14  0x79
            MX6UL_PAD_LCD_DATA15__LCDIF_DATA15  0x79
            MX6UL_PAD_LCD_DATA16__LCDIF_DATA16  0x79
            MX6UL_PAD_LCD_DATA17__LCDIF_DATA17  0x79
            MX6UL_PAD_LCD_DATA18__LCDIF_DATA18  0x79
            MX6UL_PAD_LCD_DATA19__LCDIF_DATA19  0x79
            MX6UL_PAD_LCD_DATA20__LCDIF_DATA20  0x79
            MX6UL_PAD_LCD_DATA21__LCDIF_DATA21  0x79
            MX6UL_PAD_LCD_DATA22__LCDIF_DATA22  0x79
            MX6UL_PAD_LCD_DATA23__LCDIF_DATA23  0x79
        >;
    };
    
    
    pinctrl_pwm1: pwm1grp {
    	fsl,pins = <
    		MX6UL_PAD_GPIO1_IO08__PWM1_OUT 0x110b0
    	>;
    };



如果修改了LCD显示屏的引脚，需要在这里修改对应的引脚。


.. code-block:: c 
    :caption: lcd背光pwm设置
    :linenos:

    backlight {
    	compatible = "pwm-backlight";   
    	pwms = <&pwm1 0 5000000>;
    	brightness-levels = <0 4 8 16 32 64 128 255>;
    	default-brightness-level = <6>;
    	status = "okay";
    };





背光引脚被复用为PWM1的输出，如果修改了背光引脚也要在这里修改使用的pwm。

LCD属性设置
^^^^^^^

通常情况我们参考官方开发板设计硬件LCD使用的引脚和官方一致即可。我们经常要修改的是LCD一些配置参数，例如分辨率、时钟、无效行数。


.. code-block:: c 
    :caption: lcd配置参数
    :linenos:

    /*-------第一组---------*/
    clock-frequency = <27000000>;
    hactive = <800>;
    vactive = <480>;
    
    /*-------第二组---------*/
    hfront-porch = <23>;
    hback-porch = <46>;
    vback-porch = <22>;
    vfront-porch = <22>;
    
    /*-------第三组---------*/
    hsync-len = <1>;
    vsync-len = <1>;
    
    /*-------第四组---------*/
    hsync-active = <0>;
    vsync-active = <0>;
    de-active = <1>;
    pixelclk-active = <0>;


配置参数可分为四组，第一组是设置分辨率和时钟。第二组设置“可视区域”，它们的缩写就是我们常说的HFP、hbp、vbp、vfp、行同步信号到第一个像素点的延时时间，单位（像素），一行的最后一个像素点到下一个行同步信号的延时时间（单位像素），帧同步信号到第一个有效行之间的时间，最后一行到下一个帧同步信号
之间的时间。第三组，设置行同步信号和帧同步信号的脉宽。第四组，设置行同步信号、帧同步信号、数据信号、像素时钟信号的极性。以上内容要根据自己使用的显示屏说明文档配置。

测试程序
~~~~

驱动加载成功后会在生成“/dev/fb0”文件，这个文件就是显示屏的设备节点文件。测试程序通过读、写这个文件测试显示是否正常。

源码位于“~/embed_linux_tutorial/base_code/add_lcd_device_tree/test_app”。

测试程序仅针对5寸800*480分辨率的显示屏。

进入源码目录，执行“make”命令即可。
