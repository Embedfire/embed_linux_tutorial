out_file_name = "test_app"

all: test_app.c oled_code_table.c
	arm-linux-gnueabihf-gcc $^ -o $(out_file_name)


.PHONY: clean
clean:
	rm $(out_file_name)

	
