#Makefile格式
#目标文件:依赖的文件
#Tab 命令1
#Tab 命令2
hello_main: hello_main.o hello_func.o
		gcc -o hello_main hello_main.o hello_func.o 

#以下是make的默认规则，下面两行可以不写
#hello_main.o: hello_main.c 
#		gcc -c hello_main.c 

#以下是make的默认规则，下面两行可以不写
#hello_func.o: hello_func.c
#		gcc -c hello_func.c 


#伪目标
.PHONY: clean
clean:
		rm -f *.o hello_main