#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


//通过宏选择要测试wait示例还是waitpid示例
#define     ALARM_DEFAULT   0
#define     ALARM_COVER     1

#if ((ALARM_DEFAULT | ALARM_COVER) == 0)
#error "must choose a function to compile!"
#endif

#if (ALARM_DEFAULT & ALARM_COVER)
#error "have and can only choose one of the functions to compile"
#endif


#if ALARM_DEFAULT

int main() 
{ 
    printf("\nthis is an alarm test function\n\n");
	alarm(5);
	sleep(20); 
	printf("end!\n"); 
	return 0; 
}

#else

int main() 
{
    unsigned int seconds;

    printf("\nthis is an alarm test function\n\n");

	seconds = alarm(20);

    printf("last alarm seconds remaining is %d! \n\n", seconds);

    printf("process sleep 5 seconds\n\n");
	sleep(5); 

    printf("sleep woke up, reset alarm!\n\n");

    seconds = alarm(5);

    printf("last alarm seconds remaining is %d! \n\n", seconds);

    sleep(20); 

	printf("end!\n"); 

	return 0; 
}

#endif

