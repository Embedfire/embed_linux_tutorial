#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void test1(int arg)
{
    int num;
    num = arg;

    printf("\t\t---我是 test%d 函数\n", num);
}

void test2(int arg)
{
    int num;
    num = arg;

    printf("\t--我是 test%d 函数\n", num);

    printf("\t-- test%d 开始调用 test1 \n", num);
    test1(1);
    printf("\t-- test%d 结束调用 test1 \n", num);

    printf("\t--结束调用 test%d \n", num);
}

void test3(int arg)
{
    int num;
    num = arg;

    printf("-我是 test%d 函数\n", num);

    printf("- test%d 开始调用 test2 \n", num);
    test2(2);
    printf("- test%d 结束调用 test1 \n", num);

    printf("-结束调用 test%d \n", num);
}

int main(void)
{
    test3(3);

    sleep(1);     // 防止进程过快退出

    return 0;
}


// int fibonacci(int n)
// {       
//     if (n == 1 || n == 2) {   
//         return 1;
//     }   

//     return fibonacci(n - 1) + fibonacci(n - 2);                                                                                                            
// }       
        
// int main()
// {       
//     int n = 10; 
//     int ret = 0;

//     ret = fibonacci(n);

//     printf("fibonacci(%d)=%d\n", n, ret);

//     return 0;
// }
