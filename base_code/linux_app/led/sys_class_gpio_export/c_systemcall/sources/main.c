#include <stdio.h>
#include "bsp_led.h"
#include <unistd.h>

/**
  * @brief  延时函数
  * @param  无
  * @retval 无
  */
void Delay()	 
{
	sleep(1);
}
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(int argc, char *argv[])
{
	printf("This is the led demo\n");
	LED_GPIO_Config();
	
	while(1){
		/*轮流显示 红绿蓝黄紫青白 颜色*/
		LED_RED;
		Delay();
		
		LED_GREEN;
		Delay();
		
		LED_BLUE;
		Delay();
		
		LED_YELLOW;
		Delay();
		
		LED_PURPLE;
		Delay();
				
		LED_CYAN;
		Delay();
		
		LED_WHITE
		Delay();
		
		LED_RGBOFF;
		Delay();
	}
   
	LED_GPIO_DeInit();
	return 0;
}

