#!/bin/sh

# set -v 

HOST=arm-linux-gnueabihf
SCRIPT_PATH=$(pwd)

#修改源码包解压后的名称
MAJOR_NAME=gcc-arm-linux-gnueabihf

#修改需要下载的源码前缀和后缀
OPENSRC_VER_PREFIX=8.3
OPENSRC_VER_SUFFIX=.0

PACKAGE_NAME=${MAJOR_NAME}-${OPENSRC_VER_PREFIX}${OPENSRC_VER_SUFFIX}

#定义压缩包名称
COMPRESS_PACKAGE=gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz

#定义编译后安装--生成的文件,文件夹位置路径
INSTALL_PATH=/opt/${PACKAGE_NAME}

#无需修改--下载地址
DOWNLOAD_LINK=https://developer.arm.com/-/media/Files/downloads/gnu-a/8.3-2019.03/binrel/${COMPRESS_PACKAGE}

#下载源码包
do_download_src () {
   echo "\033[1;33mstart download ${COMPRESS_PACKAGE}...\033[0m"

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

   mkdir -p ${INSTALL_PATH}

   if [ ! -d "${PACKAGE_NAME}" ];then
      tar -xf ${COMPRESS_PACKAGE} -C ${INSTALL_PATH} --strip-components=1 
   fi

   echo "\033[1;33mdone...\033[0m"
}

#删除下载的文件
do_delete_file () {
   cd ${SCRIPT_PATH}
   if [ -f "${PACKAGE_NAME}" ];then
      sudo rm -f ${PACKAGE_NAME}
   fi
}

do_download_src
do_tar_package
# do_delete_file

exit $?
