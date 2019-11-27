#生成可执行文件的名称
Target = mpu_demo
ARCH = ARM
#编译器CC
#根据传入的参数ARCH，确定使用的编译器
#默认使用arm-gcc编译器
ifeq ($(ARCH), ARM)
	CC = arm-linux-gnueabi-gcc
else
	CC = gcc
endif
#存放中间文件的路径
build_dir = objs
#存放源文件的文件夹
src_dir = src user
#存放头文件的文件夹
inc_dir = inc
#存放可执行文件的文件夹
exe_dir = bin
#源文件
sources = $(foreach dir,$(src_dir),$(wildcard $(dir)/*.c))
#目标文件（*.o）
objects = $(patsubst %.c,$(build_dir)/%.o,$(notdir $(sources)))
#头文件
includes = $(foreach dir,$(inc_dir),$(wildcard $(dir)/*.h))
#编译参数
#指定头文件的路径
CFLAGS = $(patsubst %, -I%, $(inc_dir))
#链接过程
#开发板上无法使用动态库，因此使用静态链接的方式
$(exe_dir)/$(Target) : $(objects)
	$(CC) $^ -o $@ --static
#编译工程
#编译src文件夹中的源文件，并将生成的目标文件放在objs文件夹中
$(build_dir)/%.o : src/%.c | Build_DIR
	$(CC) -c $(CFLAGS) $^ -o $@ 
#编译user文件夹中的源文件，并将生成的目标文件放在objs文件夹中
$(build_dir)/%.o : user/%.c | Build_DIR
	$(CC) -c $(CFLAGS) $^ -o $@ 
#以下为伪目标，调用方式：make 伪目标
#clean：用于Clean Project
#check：用于检查某个变量的值
.PHONY:clean check
#命令前带“-”，表示出错之后，继续执行后面的命令
clean:
	-rm -rf objs
	-rm -rf bin

#命令前带“@”,表示不在终端上输出执行的命令
check:
	@echo $(CFLAGS)
#创建一个新目录objs，用于存放过程文件
Build_DIR:
	@mkdir -p $(build_dir)
	@mkdir -p $(exe_dir)
