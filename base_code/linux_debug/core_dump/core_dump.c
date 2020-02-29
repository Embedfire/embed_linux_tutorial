#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int *a = NULL;

    printf("这是一个错误\n");

    // abort();
    *a = 0x1;

    printf("看看我是否能打印出来\n");

    sleep(1);       // 防止进程过快退出

    return 0;
}

