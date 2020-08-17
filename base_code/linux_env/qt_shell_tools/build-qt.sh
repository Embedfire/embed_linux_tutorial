#!/bin/sh

# set -v 

PLATFORM=my-linux-arm-qt
SCRIPT_PATH=$(pwd)

#修改源码包解压后的名称
MAJOR_NAME=qt-everywhere-src

#修改需要下载的源码前缀和后缀
OPENSRC_VER_PREFIX=5.11
OPENSRC_VER_SUFFIX=.3



#添加tslib交叉编译的动态库文件和头文件路径
TSLIB_LIB=/opt/tslib-1.21/lib
TSLIB_INC=/opt/tslib-1.21/include

#添加alsa交叉编译的动态库文件和头文件路径
ALSA_LIB=/opt/alsa-lib-1.2.2/lib
ALSA_INC=/opt/alsa-lib-1.2.2/include

#修改源码包解压后的名称
PACKAGE_NAME=${MAJOR_NAME}-${OPENSRC_VER_PREFIX}${OPENSRC_VER_SUFFIX}

#定义编译后安装--生成的文件,文件夹位置路径
INSTALL_PATH=/opt/${PACKAGE_NAME}

#添加交叉编译工具链路径
# CROSS_CHAIN_PREFIX=/opt/arm-gcc/bin/arm-linux-gnueabihf
CROSS_CHAIN_PREFIX=/opt/gcc-arm-linux-gnueabihf-8.3.0/bin/arm-linux-gnueabihf

#定义压缩包名称
COMPRESS_PACKAGE=${PACKAGE_NAME}.tar.xz

#无需修改--自动组合下载地址
OPENSRC_VER=${OPENSRC_VER_PREFIX}${OPENSRC_VER_SUFFIX}

case ${OPENSRC_VER_PREFIX} in
      5.9 | 5.12 | 5.13 | 5.14 |5.15 )
        DOWNLOAD_LINK=http://download.qt.io/official_releases/qt/${OPENSRC_VER_PREFIX}/${OPENSRC_VER}/single/${COMPRESS_PACKAGE} 
        ;;
    *)
        DOWNLOAD_LINK=http://download.qt.io/new_archive/qt/${OPENSRC_VER_PREFIX}/${OPENSRC_VER}/single/${COMPRESS_PACKAGE} 
        ;;
esac

#无需修改--自动组合平台路径
CONFIG_PATH=${SCRIPT_PATH}/${PACKAGE_NAME}/qtbase/mkspecs/${PLATFORM}

#无需修改--自动组合配置平台路径文件
CONFIG_FILE=${CONFIG_PATH}/qmake.conf

#下载源码包
do_download_src () {
   echo "\033[1;33mstart download ${PACKAGE_NAME}...\033[0m"

   if [ ! -f "${COMPRESS_PACKAGE}" ];then
      if [ ! -d "${PACKAGE_NAME}" ];then
         wget -c ${DOWNLOAD_LINK}
      fi
   fi

   echo "\033[1;33mdone...\033[0m"
}

#解压源码包
do_tar_package () {
   echo "\033[1;33mstart unpacking the ${PACKAGE_NAME} package ...\033[0m"
   if [ ! -d "${PACKAGE_NAME}" ];then
      tar -xf ${COMPRESS_PACKAGE}
   fi
   echo "\033[1;33mdone...\033[0m"
   cd ${PACKAGE_NAME}

   # 修复5.11.3 版本的bug
   if [ ${OPENSRC_VER_PREFIX}=="5.11" -a ${OPENSRC_VER_SUFFIX}==".3" ]; then
      sed 's/asm volatile /asm /' -i qtscript/src/3rdparty/javascriptcore/JavaScriptCore/jit/JITStubs.cpp
   fi
}

#安装依赖项
do_install_config_dependent () {
   sudo apt install g++ make qt3d5-dev-tools -y
   sudo apt install qml-module-qtquick-xmllistmodel -y
   sudo apt install qml-module-qtquick-virtualkeyboard qml-module-qtquick-privatewidgets qml-module-qtquick-dialogs qml -y
   sudo apt install libqt53dquickscene2d5 libqt53dquickrender5 libqt53dquickinput5 libqt53dquickextras5 libqt53dquickanimation5 libqt53dquick5 -y
   sudo apt install qtdeclarative5-dev qml-module-qtwebengine qml-module-qtwebchannel qml-module-qtmultimedia qml-module-qtaudioengine -y
}

#修改配置平台
do_config_before () {
   echo "\033[1;33mstart configure platform...\033[0m"

if [ ! -d "${CONFIG_PATH}" ];then
   cp -a ${SCRIPT_PATH}/${PACKAGE_NAME}/qtbase/mkspecs/linux-arm-gnueabi-g++ ${CONFIG_PATH}
fi

   echo "#" > ${CONFIG_FILE}
   echo "# qmake configuration for building with arm-linux-gnueabi-g++" >> ${CONFIG_FILE}
   echo "#" >> ${CONFIG_FILE}
   echo "" >> ${CONFIG_FILE}
   echo "MAKEFILE_GENERATOR      = UNIX" >> ${CONFIG_FILE}
   echo "CONFIG                 += incremental" >> ${CONFIG_FILE}
   echo "QMAKE_INCREMENTAL_STYLE = sublib" >> ${CONFIG_FILE}
   echo "" >> ${CONFIG_FILE}
   echo "include(../common/linux.conf)" >> ${CONFIG_FILE}
   echo "include(../common/gcc-base-unix.conf)" >> ${CONFIG_FILE}
   echo "include(../common/g++-unix.conf)" >> ${CONFIG_FILE}
   echo "" >> ${CONFIG_FILE}
   echo "# modifications to g++.conf" >> ${CONFIG_FILE}
   echo "QMAKE_CC                = ${CROSS_CHAIN_PREFIX}-gcc -lts" >> ${CONFIG_FILE}
   echo "QMAKE_CXX               = ${CROSS_CHAIN_PREFIX}-g++ -lts" >> ${CONFIG_FILE}
   echo "QMAKE_LINK              = ${CROSS_CHAIN_PREFIX}-g++ -lts" >> ${CONFIG_FILE}
   echo "QMAKE_LINK_SHLIB        = ${CROSS_CHAIN_PREFIX}-g++ -lts" >> ${CONFIG_FILE}
   echo "" >> ${CONFIG_FILE}
   echo "# modifications to linux.conf" >> ${CONFIG_FILE}
   echo "QMAKE_AR                = ${CROSS_CHAIN_PREFIX}-ar cqs" >> ${CONFIG_FILE}
   echo "QMAKE_OBJCOPY           = ${CROSS_CHAIN_PREFIX}-objcopy" >> ${CONFIG_FILE}
   echo "QMAKE_NM                = ${CROSS_CHAIN_PREFIX}-nm -P" >> ${CONFIG_FILE}
   echo "QMAKE_STRIP             = ${CROSS_CHAIN_PREFIX}-strip" >> ${CONFIG_FILE}
   echo "load(qt_config)" >> ${CONFIG_FILE}
   echo "" >> ${CONFIG_FILE}
   echo "QMAKE_INCDIR=${TSLIB_INC}" >> ${CONFIG_FILE}
   echo "QMAKE_LIBDIR=${TSLIB_LIB}" >> ${CONFIG_FILE}

   cat ${CONFIG_FILE}
   echo "\033[1;33mdone...\033[0m"
}

#配置选项
do_configure () {
   echo "\033[1;33mstart configure ${PACKAGE_NAME}...\033[0m"

   export CC="${CROSS_CHAIN_PREFIX}-gcc"
   export CXX="${CROSS_CHAIN_PREFIX}-g++" 

   ./configure \
   -prefix ${INSTALL_PATH} \
   -xplatform ${PLATFORM} \
   -release \
   -opensource \
   -confirm-license \
   -no-openssl \
   -no-opengl \
   -no-xcb \
   -no-eglfs \
   -no-compile-examples \
   -no-pkg-config \
   -skip qtquickcontrols \
   -skip qtquickcontrols2 \
   -skip qtsensors \
   -skip qtdoc \
   -skip qtwayland \
   -skip qt3d \
   -skip qtcanvas3d \
   -skip qtpurchasing \
   -skip qtcharts \
   -skip qtdeclarative \
   -no-iconv \
   -no-glib \
   -tslib \
   -I"${TSLIB_INC}" \
   -L"${TSLIB_LIB}" \
   -alsa \
   -I"${ALSA_INC}" \
   -L"${ALSA_LIB}" \

   echo "\033[1;33mdone...\033[0m"
}


#编译并且安装
do_make_install () {
   echo "\033[1;33mstart make and install ${PACKAGE_NAME} ...\033[0m"
   make && make install
   echo "\033[1;33mdone...\033[0m"
}

#删除下载的文件
do_delete_file () {
   cd ${SCRIPT_PATH}
   if [ -f "${COMPRESS_PACKAGE}" ];then
      sudo rm -f ${COMPRESS_PACKAGE}
   fi
}

do_download_src
do_tar_package
do_install_config_dependent
do_config_before
do_configure
do_make_install
# do_delete_file

exit $?
