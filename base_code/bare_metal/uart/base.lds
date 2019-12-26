ENTRY(_start)
SECTIONS {
	. = 0x80002000;
	
	. = ALIGN(4);
	.text :
	{
	start.o (.text)
	*(.text)
	}

	. = ALIGN(4);
	.data : 
	{
	*(.data)
	}
	
	. = ALIGN(4);
	.bss : 
	{
	*(.bss) 
	}
}