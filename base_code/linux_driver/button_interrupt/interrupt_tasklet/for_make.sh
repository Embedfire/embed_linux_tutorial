#! /bin/bash

error_data=-1
commend=a

#目标地址
destination_location="/home/gulun/nfs_share"


#当前路径
source_location=$(pwd)

#文件名
source_file_name="test_app.c"
file_name="test_app"

# clean old file
if [ -e ./*.ko ]; then
    make clean >/dev/null
    error_data=$?
    if [ 0 -eq $error_data ]; then
        echo -e "\033[32mclean success\033[0m"
    fi
fi

# build dirver  >/dev/null
if [ -e ./Makefile ]; then
    make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-   >/dev/null
    error_data=$?
    if [ 0 -eq $error_data ]; then
        # echo "Make success"
        echo -e "\033[32mMake success\033[0m"
    fi
else
    echo "Make file not find!!"
fi

# copy ".ko" file to 
if [ -e ./*.ko ]; then
    cp ./*.ko $destination_location
    error_data=$?
    if [ 0 -eq $error_data ]; then
        echo -e "\033[32mcopy success\033[0m"
    fi
fi






# 提示是否编译测试应用程序
# echo -e "\033[35mbuild test app , yes or no \033[0m" 
# until [ $commend = 'y' ] || [ $commend = 'n' ]; do
#     if [ $commend = 'q' ]; then
#         echo "exit"
#         exit 0
#     fi
#     echo -e "\033[35mplease insert 'y'or'n' insert 'q' exit \033[0m" 
#     read commend
# done

commend="y"

if [ $commend = "y" ]; then
    echo "comment is $commend"

    make clean -C  $source_location/test_app  
    make -C $source_location/test_app
    
    #再次检查是否存在,并将生成的文件拷贝到指定路径
    if [ -e $source_location/test_app/test_app ]; then
        cp -f $source_location/test_app/test_app $destination_location
        echo -e "\033[32mbuild test_app success \033[0m"
        echo -e "\033[32mcopy to $destination_location success  \033[0m"
    else
        echo -e "\033[31m test_app not found,test_app build error \033[0m" 
    fi

fi



if [ $commend = "n" ]; then
    echo "comment is $commend"
fi





