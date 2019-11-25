#Makefile格式
#目标:依赖
#Tab 命令1
#Tab 命令2

#默认目标
#hello_main依赖于hello_main.c和hello_func.c文件
hello_main: hello_main.c hello_func.c
		gcc -o hello_main hello_main.c hello_func.c -I .


#clean伪目标，用来删除编译生成的文件
.PHONY:clean
clean:
		rm -f *.o hello_main