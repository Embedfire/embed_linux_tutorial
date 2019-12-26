all: start.o button.o 
	arm-none-eabi-ld -Tbutton.lds  $^ -o button.elf
	arm-none-eabi-objcopy -O binary -S -g button.elf button.bin
	
%.o : %.S
	arm-none-eabi-gcc -g -c $^ -o start.o
%.o : %.c
	arm-none-eabi-gcc -g -c $^ -o button.o


.PHONY: clean
clean:
	rm *.o *.elf *.bin
	
