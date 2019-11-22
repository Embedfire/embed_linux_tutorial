#生成的可执行文件名

hello:hello.o
	arm-linux-gnueabihf-gcc -o hello hello.o
hello.o:hello.c
	arm-linux-gnueabihf-gcc -c hello.c -o hello.o

#伪目标
.PHONY:clean
clean:
	rm -f *.o hello
