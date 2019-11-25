#ifndef  _BSP_UART_H_
#define _BSP_UART_H_

extern int set_parity(int fd,int databits,int stopbits,int parity);
extern void set_speed(int fd, int speed);
#endif /* _BSP_UART_H_ */