#生成的可执行文件名

hello:hello.o
	gcc -o hello hello.o

#生成规则
hello.o:hello.c
	gcc -c hello.c -o hello.o

#伪目标
.PHONY:clean
clean:
	rm -f *.o hello