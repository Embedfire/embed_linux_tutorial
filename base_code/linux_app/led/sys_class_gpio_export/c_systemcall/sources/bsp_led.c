

#include "bsp_led.h"

/**
  * @brief  GPIO初始化函数
  * @param  无
  * @retval 无
  */
	
void LED_GPIO_Config(void)
{
	//Init GPIO for Red
	system("echo 4 > /sys/class/gpio/export");
	system("echo 'out' > /sys/class/gpio/gpio4/direction");
	LED1(OFF);
	//Init GPIO for Green
	system("echo 116 > /sys/class/gpio/export");
	system("echo 'out' > /sys/class/gpio/gpio116/direction");
	LED2(OFF);
	//Init GPIO for Blue
	system("echo 115 > /sys/class/gpio/export");
	system("echo 'out' > /sys/class/gpio/gpio115/direction");
	LED3(OFF);
}

/**
  * @brief  GPIO复位函数
  * @param  无
  * @retval 无
  */

void LED_GPIO_DeInit(void)
{
	system("echo 4 > /sys/class/gpio/unexport");

	system("echo 116 > /sys/class/gpio/unexport");

	system("echo 115 > /sys/class/gpio/unexport");
}