#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "includes/bsp_beep.h"


int beep_init(void)
{
	int fd;
	//index config
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd < 0)
		return 1 ;

	write(fd, BEEP_GPIO_INDEX, strlen(BEEP_GPIO_INDEX));
	close(fd);

	//direction config
	fd = open("/sys/class/gpio/gpio" BEEP_GPIO_INDEX "/direction", O_WRONLY);
	if(fd < 0)
		return 2;

	write(fd, "out", strlen("out"));
	close(fd);	
	
	return 0;
}

int beep_deinit(void)
{
	int fd;
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if(fd < 0)
		return 1;

	write(fd, BEEP_GPIO_INDEX, strlen(BEEP_GPIO_INDEX));
	close(fd);
	
	return 0;
}


int beep_on(void)
{
	int fd;

	fd = open("/sys/class/gpio/gpio" BEEP_GPIO_INDEX "/value", O_WRONLY);
	if(fd < 0)
		return 1;

	write(fd, "1", 1);
	close(fd);

	return 0;
}

int beep_off(void)
{
	int fd;

	fd = open("/sys/class/gpio/gpio" BEEP_GPIO_INDEX "/value", O_WRONLY);
	if(fd < 0)
		return 1;

	write(fd, "0", 1);
	close(fd);

	return 0;
}


