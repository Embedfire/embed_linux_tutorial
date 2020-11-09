# i.mx6ul_board
This is an linux demo code for the i.mx6ul development board.

Makefile uses static linking, if you do not use static linking:
  change `FLAG = -g -Werror -I. -Iinclude -static` to `FLAG = -g -Werror -I. -Iinclude`

alarm       通过alarm发送信号
exec        exec函数替换进程示例
fifo        fifo缓冲区使用示例
file        创建文件示例
file_lock   多进程读写文件示例
fork        通过fork函数启动新进程示例
kill        使用kill终止进程
msg         消息队列使用示例
mutex       互斥锁使用示例
pipe        pipe管道使用示例
posix_sem   posix无名信号量使用示例
posix_sem1  posix有名信号量使用示例
process     使用fork的进程控制示例
shm_read    共享内存读取示例
shm_write   共享内存写入示例
sigaction   通过sigaction函数捕获信号
signal      使用signal函数捕获信号
stream_pipe 文件流管道使用示例
system      通过system函数启动新进程示例
systemV_sem system-V IPC信号量示例
tcp_client  tcp客户端示例
tcp_server  tcp服务端示例
thread      线程控制实验
thread_attr 线程属性控制实验
vfork       通过vfork函数启动新进程示例
wait        通过wait函数阻塞进程的示例
