obj-m				+= mcp25xxfd-can.o
mcp25xxfd-can-objs                  := mcp25xxfd_base.o
mcp25xxfd-can-objs                  += mcp25xxfd_can.o
mcp25xxfd-can-objs                  += mcp25xxfd_can_debugfs.o
mcp25xxfd-can-objs                  += mcp25xxfd_can_fifo.o
mcp25xxfd-can-objs                  += mcp25xxfd_can_int.o
mcp25xxfd-can-objs                  += mcp25xxfd_can_rx.o
mcp25xxfd-can-objs                  += mcp25xxfd_can_tx.o
mcp25xxfd-can-objs                  += mcp25xxfd_clock.o
mcp25xxfd-can-objs                  += mcp25xxfd_cmd.o
mcp25xxfd-can-objs                  += mcp25xxfd_crc.o
mcp25xxfd-can-objs                  += mcp25xxfd_debugfs.o
mcp25xxfd-can-objs                  += mcp25xxfd_ecc.o
mcp25xxfd-can-objs                  += mcp25xxfd_gpio.o
mcp25xxfd-can-objs                  += mcp25xxfd_int.o




all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo cp mcp25xxfd-can.ko /lib/modules/$(shell uname -r)/kernel/drivers/net/can/spi/
	sudo depmod -a
