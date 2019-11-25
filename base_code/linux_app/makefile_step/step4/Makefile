#Makefile格式
#目标文件:依赖的文件
#Tab 命令1
#Tab 命令2

#定义变量
#ARCH默认为x86，使用gcc编译器，
#否则使用ARM编译器
ARCH ?= x86
TARGET = hello_main
CFLAGS = -I.
DEPS = hello_func.h
OBJS = hello_main.o hello_func.o


#根据输入的ARCH变量来选择编译器
#ARCH=x86，使用gcc
#ARCH=arm，使用arm-gcc
ifeq ($(ARCH),x86)
  CC = gcc
else
  CC = arm-linux-gnueabihf-gcc
endif

#目标文件
$(TARGET): $(OBJS)
		$(CC) -o $@ $^ $(CFLAGS) 

#*.o文件的生成规则
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#伪目标
.PHONY: clean
clean:
		rm -f *.o hello_main