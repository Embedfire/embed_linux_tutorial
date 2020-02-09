all: start.o led.o 
	arm-none-eabi-ld -Tled.lds  $^ -o led.elf
	arm-none-eabi-objcopy -O binary -S -g led.elf led.bin
	
%.o : %.S
	arm-none-eabi-gcc -g -c $^ -o start.o
%.o : %.c
	arm-none-eabi-gcc -g -c $^ -o led.o


.PHONY: clean
clean:
	rm *.o *.elf *.bin
	
