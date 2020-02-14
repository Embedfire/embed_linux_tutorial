#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>


// int main() 
// { 
//     printf("\nthis is an alarm test function\n\n");
// 	alarm(5);
// 	sleep(20); 
// 	printf("end!\n"); 
// 	return 0; 
// }


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


