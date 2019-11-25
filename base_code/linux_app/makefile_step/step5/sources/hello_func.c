/* $begin hello_func */
#include <stdio.h>
#include "hello_func.h"

void hello_func(void) 
{
	printf("hello, world! This is a C program.\n");
	for(int i=0;i<10;i++ )
	{
		printf("output i=%d\n",i);
	}
}
/* $end hello_func */

