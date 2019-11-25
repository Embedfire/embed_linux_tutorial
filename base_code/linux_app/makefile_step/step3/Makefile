#Makefile格式
#目标文件:依赖的文件
#Tab 命令1
#Tab 命令2

#定义变量
CC = gcc
CFLAGS = -I.
DEPS = hello_func.h


#目标文件
hello_main: hello_main.o hello_func.o
		$(CC) -o hello_main hello_main.o hello_func.o 

#*.o文件的生成规则
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#伪目标
.PHONY: clean
clean:
		rm -f *.o hello_main