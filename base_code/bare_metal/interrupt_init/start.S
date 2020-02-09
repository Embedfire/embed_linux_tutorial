/*DDR的前8K字节保留， [0x80000000-0x80001FFF] 保留为ROM区域 */

/*定义内存起始地址和大小*/
#define m_DDR_start             0x80000000
#define m_DDR_size              0x20000000

/*定义主代码区域，m_text_start将会作为中断向量表的起始地址，链接脚本中
*将该地址用作起始链接地址。
*/
#define  m_text_start           0x80002000

/*定义Supervisor工作模式的栈起始地址和大小
*野火开发板标配512M字节的DDR, Supervisor工作模式的栈和IRQ工作模式的栈
*位于DDR的后2M地址，大小均为1M。
*/
#define   SUP_model_stack_start     0x9FE00000
#define   SUP_model_stack_size      0x00100000


/*定义IRQ工作模式的栈起始地址和大小，大小为1M*/
#define   IRQ_model_stack_start     0x9FF00000
#define   IRQ_model_stack_size      0x00100000



.globl light_led

.text
.align 2         //设置字节对齐
.global _start
_start:

    ldr     pc, =Reset_Handler           /* Reset                  */
    ldr     pc, =Undefined_Handler       /* Undefined instructions */
    ldr     pc, =SVC_Handler             /* Supervisor Call        */
    ldr     pc, =PrefAbort_Handler       /* Prefetch abort         */
    ldr     pc, =DataAbort_Handler       /* Data abort             */
    .word   0                            /* RESERVED               */
    ldr     pc, =IRQ_Handler             /* IRQ interrupt          */
    ldr     pc, =FIQ_Handler             /* FIQ interrupt          */



Reset_Handler:
    cpsid   i                         /* 全局关闭中断 */


    mrc     p15, 0, r0, c1, c0, 0     /*读取CP15系统控制寄存器   */
    bic     r0,  r0, #(0x1 << 12)     /*  清除第12位（I位）禁用 I Cache  */
    bic     r0,  r0, #(0x1 <<  2)     /*  清除第 2位（C位）禁用 D Cache  */
    bic     r0,  r0, #0x2             /*  清除第 1位（A位）禁止严格对齐   */
    bic     r0,  r0, #(0x1 << 11)     /*  清除第11位（Z位）分支预测   */
    bic     r0,  r0, #0x1             /*  清除第 0位（M位）禁用 MMU   */
    mcr     p15, 0, r0, c1, c0, 0     /*  将修改后的值写回CP15寄存器   */

    /* 定义IRQ工作模式的栈起始地址 */
    cps     #0x12                
    ldr     sp, =IRQ_model_stack_start    
    
    /*定义User工作模式的栈起始地址，与Supervisor相同*/
    cps     #0x1F               
    ldr     sp, =SUP_model_stack_start    

    /*定义Supervisor工作模式的栈起始地址，与User相同 */
    cps     #0x13                
    ldr     sp, =SUP_model_stack_start   

    /*跳转到系统初始化函数，初始化GIC、CACHE-L1、mmu等等*/
    ldr     r2, =SystemInit      
    blx     r2  
    
    /*开启全局中断*/
    cpsie   i                   
    
    /*跳转到到 main 函数执行，*/
    b main                
    b .        /*死循环*/





Undefined_Handler:
    b Undefined_Handler
    .size Undefined_Handler, . - Undefined_Handler

    .align 2
    .arm
    .weak SVC_Handler
    .type SVC_Handler, %function
SVC_Handler:
    ldr   r0,=SVC_Handler
    bx    r0
    .size SVC_Handler, . - SVC_Handler

    .align 2
    .arm
    .weak PrefAbort_Handler
    .type PrefAbort_Handler, %function
PrefAbort_Handler:
    ldr   r0,=PrefAbort_Handler
    bx    r0
    .size PrefAbort_Handler, . - PrefAbort_Handler

    .align 2
    .arm
    .weak DataAbort_Handler
    .type DataAbort_Handler, %function
DataAbort_Handler:
    ldr   r0,=DataAbort_Handler
    bx    r0
    .size DataAbort_Handler, . - DataAbort_Handler

    .align 2
    .arm
    .weak IRQ_Handler
    .type IRQ_Handler, %function
IRQ_Handler:
    push    {lr}                         /* Save return address+4                                */
    push    {r0-r3, r12}                 /* Push caller save registers                           */

    MRS     r0, spsr                     /* Save SPRS to allow interrupt reentry                 */
    push    {r0}

    MRC     P15, 4, r1, C15, C0, 0       /* Get GIC base address  */
    ADD     r1, r1, #0x2000              /* r1: GICC base address  */
    LDR     r0, [r1, #0xC]               /* r0: IAR  */

    push    {r0, r1}

    CPS     #0x13                        /* Change to Supervisor mode to allow interrupt reentry */

    push    {lr}                         /* Save Supervisor lr  */
    ldr     r2, =SystemIrqHandler
    blx     r2
                           
    POP     {lr}

    CPS     #0x12                        /* Back to IRQ mode                                     */

    POP     {r0, r1}

    STR     r0, [r1, #0x10]              /* Now IRQ handler finished: write to EOIR              */

    POP     {r0}
    MSR     spsr_cxsf, r0

    POP     {r0-r3, r12}
    POP     {lr}
    SUBS    pc, lr, #4
    .size IRQ_Handler, . - IRQ_Handler

    .align 2
    .arm
    .weak FIQ_Handler
    .type FIQ_Handler, %function


FIQ_Handler:
    ldr   r0,=FIQ_Handler
    bx    r0
    .size FIQ_Handler, . - FIQ_Handler

    .end








