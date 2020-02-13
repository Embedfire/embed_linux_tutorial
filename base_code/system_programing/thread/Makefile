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
CC=gcc
SRC = $(wildcard *.c */*.c)
OBJS = $(patsubst %.c, %.o, $(SRC))
DEP_FILES := $(patsubst %, .%.d,$(OBJS))
DEP_FILES := $(wildcard $(DEP_FILES))
FLAG = -g -Werror -I. -Iinclude  -lpthread -static
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
