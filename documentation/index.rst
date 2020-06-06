.. [野火]i.MX Linux开发实战指南 documentation master file, created by
   sphinx-quickstart on Wed Nov 20 09:46:40 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

[野火]i.MX Linux开发实战指南
========================================================

.. toctree::
   :maxdepth: 1


   README
   about_us
   TODO

.. toctree::
   :maxdepth: 1
   :caption: 熟悉linux操作系统
   :numbered:

   linux_basis/why_learning_linux
   linux_basis/how_to_learn_linux
   linux_basis/i_mx_series
   linux_basis/linux_brief
   linux_basis/installing_linux
   linux_basis/linux_directory
   linux_basis/user_group
   linux_basis/command_line
   linux_basis/software_package
   linux_basis/editor
   #linux_basis/git_tutorial
   

.. toctree::
   :maxdepth: 1
   :caption: 熟悉EBF_6ULL开发板
   :numbered:

   linux_basis/ebf6ull_hardware
   install_image/install_debian_to_sd
   linux_basis/board_startup
   linux_basis/system_information
   linux_basis/fire-config_brief
   linux_basis/led_key_command_line_testing
   linux_basis/script_hardware_testing
   linux_basis/ec20_4g_module

.. toctree::
   :maxdepth: 1
   :caption: 在Linux下开发应用程序
   :numbered:

   linux_app/git_use
   linux_app/mount_nfs
   linux_app/gcc_hello_world
   linux_app/arm_gcc_hello_world.rst
   linux_app/hello_in_linux.rst
   linux_app/Makefile_brife.rst
   linux_app/makefile
   linux_app/filesystem_system_call
   linux_app/led_subsystem
   linux_app/gpio_subsystem
   linux_app/input_subsystem
   linux_app/uart_tty
   linux_app/i2c_bus
   linux_app/spi_bus

.. toctree::
   :maxdepth: 1
   :caption: 制作系统镜像
   :numbered:


   building_image/image_composition
   building_image/building_NXP_firmware
   building_image/burning_NXP_firmware
   building_image/building_debian


   #building_image/building_kernel
   #building_image/building_uboot
   #building_image/building_rootfs
   #building_image/using_buildroot
   #building_image/using_yocto
   #building_image/building_debian

.. toctree::
   :maxdepth: 1
   :caption: 环境搭建
   :numbered:

   linux_env/qt_cross_compiling
   linux_env/python37_env
   linux_env/pyqt5_env
   linux_env/python_ssd1306
   linux_env/python_gpio
   linux_env/python_serial
   linux_env/using_sqlite
   linux_env/using_qtsqlite
   linux_env/using_php

.. toctree::
   :maxdepth: 1
   :caption: Linux系统编程
   :numbered:

   system_programing/process
   system_programing/pipe
   system_programing/signal
   system_programing/msg_queuq
   system_programing/systemV_sem
   system_programing/shm
   system_programing/thread
   system_programing/posix_sem
   system_programing/posix_mutex
   system_programing/tcp_ip
   system_programing/socket

.. toctree::
   :maxdepth: 1
   :caption: Linuxn内核调试
   :numbered:

   linux_debug/gdb_use
   linux_debug/core_dump
   linux_debug/backtrace
   linux_debug/strace

.. toctree::
   :maxdepth: 1
   :caption: Linux物联网编程
   :numbered:

   linux_iot/linux_http
   linux_iot/linux_http_client
   linux_iot/linux_http_server
   linux_iot/canopen
   linux_iot/modbus
   linux_iot/ATGM332D
   linux_iot/bluetooth
   linux_iot/mqtt
   linux_iot/mqtt-client
   linux_iot/mqtt-baidu
   linux_iot/mqtt-onenet


.. toctree::
   :maxdepth: 1
   :caption: i.MX6 裸机开发
   :numbered:

   bare_metal/sdk
   bare_metal/building_sdk_demo
   bare_metal/before_developing
   bare_metal/asemble_led
   bare_metal/iar_led
   bare_metal/gcc_led
   bare_metal/beautify_led
   bare_metal/gpio_key
   bare_metal/sorting_project
   bare_metal/interrupt
   bare_metal/ccm
   bare_metal/uart
   bare_metal/ddr
   bare_metal/elcdif

.. toctree::
   :maxdepth: 1
   :caption: Linux驱动开发
   :numbered:

   linux_driver/module
   linux_driver/character_device
   linux_driver/led_character_device
   linux_driver/linux_device_model
   linux_driver/platform_driver
   linux_driver/driver_tree
   linux_driver/device_tree_rgb_led
   linux_driver/dynamic_device_tree
   linux_driver/gpio_subsystem
   linux_driver/concurrency
   linux_driver/i2c_mpu6050.rst
   linux_driver/ecspi_oled.rst
   
   

.. toctree::
   :maxdepth: 1
   :caption: 附录
   :numbered:

   appendix/install_sd_image

.. toctree::
   :maxdepth: 1
   :caption: 投稿精选
   :titlesonly:

   submission/rst-testfile
   submission/ubuntu16-rootfs
   submission/debian9-rootfs
   submission/野火IMX6UL开发板SDK使用说明
   submission/buildroot2017-1
   submission/buildroot2017-2
   submission/buildroot2017-3


.. toctree::
   :maxdepth: 1
   :caption: 贡献与投稿
   :numbered:

   contribute/how_to_contribute


.. toctree::
   :maxdepth: 2
   :caption: 常见问题

   faq/index

.. toctree::
   :maxdepth: 1
   :caption: 版权

   LICENSE



