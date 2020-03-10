/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-02-10 16:51:42
 * @LastEditTime: 2020-02-10 17:32:31
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */
/* vfork.c */


#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main(void)
{
    pid_t result;
    /*调用 fork()函数*/
    result = vfork();
    /*通过 result 的值来判断 fork()函数的返回情况，首先进行出错处理*/
    if(result == -1)
    {
        printf("Fork error\n");
    }
    else if (result == 0) /*返回值为 0 代表子进程*/
    {
        printf("The returned value is %d\nIn child process!!\nMy PID is %d\n\n",result,getpid());

    }
    else /*返回值大于 0 代表父进程*/
    {
        printf("The returned value is %d\nIn father process!!\nMy PID is %d\n\n",result,getpid());
    }
    exit(1);
}
