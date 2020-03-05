<<<<<<< HEAD
# 解释一下： 
# 　　（1）第1行指定编译器为 gcc，可以根据需要修改为 g++ 或者 arm-linux-gcc 等交叉编译工具链。 
# 　　（2）第2行是为了获取匹配模式的文件名，*.c 表示当前工程目录的 c 文件，*/*.c 表示所有下一级目录的 .c 文件。 
# 　　（3）第3行是将 $(SRC) 中的 .c 文件替换成 .o 文件。 	SRC=$(wildcard *.c */*.c)
# 　　（4）第4行用于指定编译选项，根据需要添加，比如 -g、-ml、-Wall、-O2 等等。 
# 　　（5）第5、6行，同样使用了 wildcard 和 patsubst 函数来处理，目的是为了指定最终生成的可执行文件名。 
# 　　（6）第8行 $(TARGET):$(OBJS) 表示由 .o 文件链接成可执行文件。 
# 　　（7）注意第9行前面是一个 <tab> 键，而 $@ 表示目标，也就是 $(TARGET)，$^ 表示依赖列表，也就是 $(OBJS) 。 
# 　　（9）第12行中的 $< 表示搜索到的第一个匹配的文件。 
# 　　（10）第14行的 clean 是伪目标（.PHONY），它不需要依赖，执行 make 命令时是不会执行的，当执行 make clean 时才会执行。 
# 　　（11）.PHONY:clean 防止当前目录下有clean文件导致无法清除
# 　　另外，Makefile 的赋值除了使用等号（=），还可以使用 := 、+= 、?= 符号。具体含义如下：

# := 即使变量   表示简单赋值（注意右边的 $ 值只会向上寻找）
# = 延时变量
# += 表示追加赋值
# ?= 表示判断赋值（判断左边是否已经定义过，如果是则忽略本次赋值）

# CC=gcc
# SRCS=$(wildcard *.c */*.c)
# OBJS=$(patsubst %.c, %.o, $(SRCS))
# FLAG=-g
# NAME=$(wildcard *.c)
# TARGET=$(patsubst %.c, %, $(NAME))

# $(TARGET):$(OBJS)
# 	$(CC) -o $@ $^ $(FLAG)

# %.o:%.c
# 	$(CC) -o $@ -c $< -g

# clean:
# 	rm -rf $(TARGET) $(OBJS)

# .PHONY:clean

# CC = arm-linux-gnueabihf-gcc
# CC=gcc
CC = arm-linux-gnueabihf-gcc
SRC = $(wildcard *.c */*.c)
OBJS = $(patsubst %.c, %.o, $(SRC))
DEP_FILES := $(patsubst %, .%.d,$(OBJS))
DEP_FILES := $(wildcard $(DEP_FILES))
FLAG = -g -Werror -I. -Iinclude -static
TARGET = targets

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(FLAG)

ifneq ($(DEP_FILES),)
include $(DEP_FILES)
endif

%.o:%.c
	$(CC) -o $@ -c $(FLAG) $< -g -MD -MF .$@.d

clean:
	rm -rf $(TARGET) $(OBJS)

distclean:
	rm -rf $(DEP_FILES)

.PHONY:clean
=======

#生成可执行文件的名称
Target = fork_demo
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


>>>>>>> 更新系统应用程序部分代码
