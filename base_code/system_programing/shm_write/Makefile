
#生成可执行文件的名称
Target = shm_write_demo
ARCH ?= x86
#编译器CC
#根据传入的参数ARCH，确定使用的编译器
#默认使用gcc编译器
#make ARCH=arm 时使用ARM-GCC编译器
ifeq ($(ARCH), x86)
	CC = gcc
else
	CC = arm-linux-gnueabihf-gcc
endif
#存放中间文件的路径
build_dir = build_$(ARCH)
#存放源文件的文件夹
src_dir =  sources
#存放头文件的文件夹
inc_dir = includes .

#源文件
sources = $(foreach dir,$(src_dir),$(wildcard $(dir)/*.c))
#目标文件（*.o）
objects = $(patsubst %.c,$(build_dir)/%.o,$(notdir $(sources)))
#头文件
includes = $(foreach dir,$(inc_dir),$(wildcard $(dir)/*.h))

#目标依赖文件
DEP_FILES := $(patsubst %, .%.d,$(objects))
DEP_FILES := $(wildcard $(DEP_FILES))

#判断依赖文件是否存在
ifneq ($(DEP_FILES),)
#如果存在依赖文件，需要导入依赖文件
include $(DEP_FILES)
endif

#编译参数
#指定头文件的路径
CFLAGS = $(patsubst %, -I%, $(inc_dir)) -MD -MF $(@D)/.$(@F).d

#链接过程
$(build_dir)/$(Target) : $(objects)  | create_build
	$(CC) $^ -o $@

#编译工程
#编译src文件夹中的源文件，并将生成的目标文件放在objs文件夹中
$(build_dir)/%.o : $(src_dir)/%.c $(includes) | create_build
	$(CC) -c $(CFLAGS) $< -o $@


#以下为伪目标，调用方式：make 伪目标
#clean：用于Clean Project
#check：用于检查某个变量的值
.PHONY:clean cleanall check create_build
#按架构删除
clean:
	rm -rf $(build_dir)

#全部删除
cleanall:
	rm -rf build_x86 build_arm

#命令前带"@",表示不在终端上输出执行的命令
#这个目标主要是用来调试Makefile时输出一些内容
check:
	@echo $(CFLAGS)
	@echo $(CURDIR)
	@echo $(src_dir)
	@echo $(sources)
	@echo $(objects)


#创建一个新目录create，用于存放过程文件
create_build:
	@mkdir -p $(build_dir)


