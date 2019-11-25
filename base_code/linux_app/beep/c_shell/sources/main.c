#include <stdio.h>
#include <unistd.h>

#include "includes/bsp_beep.h"

/**
	* @brief  主函数
	* @param  无
	* @retval 无
	*/
int main(int argc, char *argv[])
{
	char buf[10];
	printf("This is the beep demo\n");
	
	beep_init();

	while(1){
		printf("Please input the value : 0--off 1--on q--exit\n");
		scanf("%10s", buf);

		switch (buf[0]){
			case '0':
				beep_off();
				break;

			case '1':
				beep_on();
				break;

			case 'q':
				beep_deinit();
				printf("Exit\n");
				return 0;

			default:
				break;
		}
	}
}


