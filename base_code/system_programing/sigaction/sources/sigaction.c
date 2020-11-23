/** signal.c */

/**
 * 一个进程可以决定在该进程中需要对哪些信号进行什么样的处理。例如，一个进程可以选择忽略某些信号而只处理其他一些信号，
 * 另外，一个进程还可以选择如何处理信号。总之，这些都是与特定的进程相联系的。因此，首先就要建立进程与其信号之间的对
 * 应关系，这就是信号的处理。
 * 
 * 如果进程要处理某一信号，那么就要在进程中安装该信号。安装信号主要用来确定信号值及进程针对该信号值的动作之间的映射关系，
 * 即进程将要处理哪个信号；该信号被传递给进程时，将执行何种操作。
 * 
 * linux主要有两个函数实现信号的安装：signal()、sigaction()。其中signal()在可靠信号系统调用的基础上实现, 是库函数。
 * 它只有两个参数，不支持信号传递信息，主要是用于前32种非实时信号的安装；
 * 而sigaction()是较新的函数（由两个系统调用实现：sys_signal以及sys_rt_sigaction），有三个参数，支持信号传递信息，主要用来与 
 * sigqueue() 系统调用配合使用，当然，sigaction()同样支持非实时信号的安装。sigaction()优于signal()主要体现在支持信号带有参数。
 * 
 * 常见信号的含义及其默认操作：
 * 信 号 名                     含 义                               默 认 操 作
 * 
 * SIGHUP       该信号在用户终端连接（正常或非正常）结束时发出，              终止
 *              通常是在终端的控 制进程结束时，通知同一会话内的      
 *              各个作业与控制终端不再关联 
 * 
 * SIGINT       该信号在用户键入 INTR 字符（通常是 Ctrl-C）时发出，         终止
 *              终端驱动程序发送此信号并送到前台进程中的每一个进程            
 * 
 * SIGQUIT      该信号和 SIGINT 类似，                                  终止
 *              但由 QUIT 字符（通常是 Ctrl-\）来控制                     
 * 
 * SIGILL       该信号在一个进程企图执行一条非法指令时（可执行文件本          终止
 *              身出现错误，或者试图执行数据段、堆栈溢出时）发出 
 * 
 * SIGFPE       该信号在发生致命的算术运算错误时发出。这里不仅包括浮点        终止  
 *              运算错误，还包括溢出及除数为 0 等其他所有的算术错误         
 * 
 * SIGKILL      该信号用来立即结束程序的运行，                            终止
 *              并且不能被阻塞、处理或忽略
 * 
 * SIGALRM      该信号当一个定时器到时的时候发出                          终止
 * 
 * SIGSTOP      该信号用于暂停一个进程，且不能被阻塞、处理或忽略            暂停进程
 * 
 * SIGTSTP      该信号用于交互停止进程，用户键入 SUSP 字符时              停止进程
 *              （通常是 Ctrl+Z）发出这个信号                       
 * 
 * SIGCHLD      子进程改变状态时，父进程会收到这个信号                     忽略
 * 
 * SIGABORT      进程异常终止时发出
 */

/** 
 * Linux 系统为大部分信号定义了缺省处理方法，当信号的缺省处理方法不满足需求时，可通过 sigaction()函数进行改变
 * 
 * 函数原型: int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
 * 
 * signum：信号代码，可以为除 SIGKILL 及 SIGSTOP 外的任何一个特定有效的信号
 * act：指向结构 sigaction 的一个实例的指针，指定对特定信号的处理
 * oldact：保存原来对相应信号的处理
 * 
 * struct sigaction {
 *  void (*sa_handler)(int);
 *  void (*sa_sigaction)(int, siginfo_t *, void *);
 *  sigset_t sa_mask;
 *  int sa_flags;
 *  void (*sa_restorer)(void);
 * };
 * 
 *   sa_handler 是一个函数指针， 用来指定信号发生时调用的处理函数；
 *   sa_sigaction 则是另外一个信号处理函数，它有三个参数，可以获得关于信号的更
 * 详细的信息； 当 sa_flags 成员的值包含了 SA_SIGINFO 标志时，系统将使用
 * sa_sigaction 函数作为信号的处理函数，否则将使用 sa_handler；
 *   sa_mask 成员用来指定在信号处理函数执行期间需要被屏蔽的信号，特别是当某个
 * 信号正被处理时，它本身会被自动地放入进程的信号掩码，因此在信号处理函数执
 * 行期间， 这个信号都不会再度发生。可以使用 sigemptyset()、 sigaddset()、 sigdelset()
 * 分别对这个信号集进行清空、增加和删除被屏蔽信号的操作；
 *   sa_flags 成员用于指定信号处理的行为，它可以是以下值的"按位或" 组合：
 *       SA_RESTART：使被信号打断的系统调用自动重新发起；
 *       SA_NOCLDSTOP：使父进程在它的子进程暂停或继续运行时不会收到SIGCHLD 信号；
 *       SA_NOCLDWAIT：使父进程在它的子进程退出时不会收到 SIGCHLD 信号，这时子进程如果退出也不会成为僵尸进程；
 *       SA_NODEFER：使对信号的屏蔽无效，即在信号处理函数的执行期间仍能发出这个信号；
 *       SA_RESETHAND：信号被处理后重新设置处理方式到默认值；
 *       SA_SIGINFO：使用 sa_sigaction 成员而不是 sa_handler 作为信号处理函数。
 *   re_restorer 成员则是一个已经废弃的数据域，不要使用。
 *  
 * retern:
 *   成功：以前的信号处理配置
 *   出错：-1
 * 
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


/** 信号处理函数 */
void signal_handler(int sig)
{
    printf("\nthis signal number is %d \n",sig);

    if (sig == SIGINT) {
        printf("I have get SIGINT!\n\n");
        printf("The signal is automatically restored to the default handler!\n\n");
        /** 信号自动恢复为默认处理函数 */
    }

}

int main(void)
{
    struct sigaction act;

    printf("this is sigaction function test demo!\n\n");

    /** 设置信号处理的回调函数 */
    act.sa_handler = signal_handler;
    
    /* 清空屏蔽信号集 */
    sigemptyset(&act.sa_mask); 

    /** 在处理完信号后恢复默认信号处理 */
    act.sa_flags = SA_RESETHAND;

    sigaction(SIGINT, &act, NULL);

    while (1)
    {
        printf("waiting for the SIGINT signal , please enter \"ctrl + c\"...\n\n");
        sleep(1);
    }
    
    exit(0);
}

