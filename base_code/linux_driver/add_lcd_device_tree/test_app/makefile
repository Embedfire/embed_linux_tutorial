out_file_name = "test_app"

all: test_app.c test_picture.c 
	arm-linux-gnueabihf-gcc $^ -o $(out_file_name)


.PHONY: clean
clean:
	rm $(out_file_name)

	
