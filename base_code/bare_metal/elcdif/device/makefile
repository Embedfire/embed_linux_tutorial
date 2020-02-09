all : button.o  led.o system_MCIMX6Y2.o clock.o uart.o elcdif.o
	arm-none-eabi-ld -r $^  -o device.o
	
%.o : %.c
	arm-none-eabi-gcc ${header_file} -c $^
	
%.o : %.S
	arm-none-eabi-gcc ${header_file} -c $^

clean:
	-rm -f *.o *.bak		





