#include <stdlib.h>
#include "bsp_beep.h"

void beep_init(void)
{
	system("echo " BEEP_GPIO_INDEX " > /sys/class/gpio/export");
	system("echo 'out' > /sys/class/gpio/gpio" BEEP_GPIO_INDEX "/direction");
}

void beep_deinit(void)
{
	system("echo " BEEP_GPIO_INDEX " > /sys/class/gpio/unexport");
	
}


void beep_on(void)
{
	system("echo 1 > /sys/class/gpio/gpio" BEEP_GPIO_INDEX "/value");
}

void beep_off(void)
{
	system("echo 0 > /sys/class/gpio/gpio" BEEP_GPIO_INDEX "/value");
}

