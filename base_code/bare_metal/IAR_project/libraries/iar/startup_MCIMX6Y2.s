; ---------------------------------------------------------------------------------------
;  @file:    startup_MCIMX6Y2.s
;  @purpose: CMSIS Cortex-A7 Core Device Startup File
;            MCIMX6Y2
;  @version: 2.0
;  @date:    2016-8-24
;  @build:   b160722
; ---------------------------------------------------------------------------------------
;
; Copyright (c) 2016 , NXP Semiconductor, Inc.
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without modification,
; are permitted provided that the following conditions are met:
;
; o Redistributions of source code must retain the above copyright notice, this list
;   of conditions and the following disclaimer.
;
; o Redistributions in binary form must reproduce the above copyright notice, this
;   list of conditions and the following disclaimer in the documentation and/or
;   other materials provided with the distribution.
;
; o Neither the name of NXP Semiconductor, Inc. nor the names of its
;   contributors may be used to endorse or promote products derived from this
;   software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
; ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; Cortex-A version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)
        SECTION ISTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)    

        PUBLIC  __vector_table

        DATA

__vector_table
        ARM
        LDR     PC, Reset_Word           ; Reset
        LDR     PC, Undefined_Word       ; Undefined instructions
        LDR     PC, SVC_Word             ; Supervisor Call
        LDR     PC, PrefAbort_Word       ; Prefetch abort
        LDR     PC, DataAbort_Word       ; Data abort
        DCD     0                        ; RESERVED
        LDR     PC, IRQ_Word             ; IRQ interrupt
        LDR     PC, FIQ_Word             ; FIQ interrupt

        DATA

Reset_Word      DCD   __iar_program_start
Undefined_Word  DCD   Undefined_Handler
SVC_Word        DCD   SVC_Handler
PrefAbort_Word  DCD   PrefAbort_Handler
DataAbort_Word  DCD   DataAbort_Handler
IRQ_Word        DCD   IRQ_Handler
FIQ_Word        DCD   FIQ_Handler

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        PUBLIC  __iar_program_start
        SECTION .text:CODE:REORDER:NOROOT(2)

        REQUIRE __vector_table
        EXTERN  __cmain
        EXTERN  SystemInit

        ARM

__iar_program_start
        CPSID   I                         ; Mask interrupts

        ; Reset SCTLR Settings
        MRC     P15, 0, R0, C1, C0, 0     ; Read CP15 System Control register
        BIC     R0,  R0, #(0x1 << 12)     ; Clear I bit 12 to disable I Cache
        BIC     R0,  R0, #(0x1 <<  2)     ; Clear C bit  2 to disable D Cache
        BIC     R0,  R0, #0x2             ; Clear A bit  1 to disable strict alignment
        BIC     R0,  R0, #(0x1 << 11)     ; Clear Z bit 11 to disable branch prediction
        BIC     R0,  R0, #0x1             ; Clear M bit  0 to disable MMU
        MCR     P15, 0, R0, C1, C0, 0     ; Write value back to CP15 System Control register

        ; Set up stack for IRQ, System/User and Supervisor Modes
        ; Enter IRQ mode
        CPS     #0x12
        LDR     SP, =SFE(ISTACK)     ; Set up IRQ handler stack

        ; Enter System mode
        CPS     #0x1F
        LDR     SP, =SFE(CSTACK)     ; Set up System/User Mode stack

        ; Enter Supervisor mode
        CPS     #0x13
        LDR     SP, =SFE(CSTACK)     ; Set up Supervisor Mode stack

        LDR     R0, =SystemInit
        BLX     R0
        CPSIE   I                    ; Unmask interrupts

        ; Application runs in Supervisor mode
        LDR     R0, =__cmain
        BX      R0

        PUBWEAK Undefined_Handler
        PUBWEAK SVC_Handler
        PUBWEAK PrefAbort_Handler
        PUBWEAK DataAbort_Handler
        PUBWEAK IRQ_Handler
        PUBWEAK FIQ_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)

        EXTERN  SystemIrqHandler

        ARM

Undefined_Handler
        B .     ; Undefined instruction at address LR-Off (Off=4 in ARM mode and Off=2 in THUMB mode)

SVC_Handler
        B .     ; Supervisor call from Address LR

PrefAbort_Handler
        B .     ; Prefetch instruction abort at address LR-4

DataAbort_Handler
        B .     ; Load data abort at instruction address LR-8

IRQ_Handler
        PUSH    {LR}                         ; Save return address+4
        PUSH    {R0-R3, R12}                 ; Push caller save registers

        MRS     R0, SPSR                     ; Save SPRS to allow interrupt reentry
        PUSH    {R0}

        MRC     P15, 4, R1, C15, C0, 0       ; Get GIC base address
        ADD     R1, R1, #0x2000              ; R1: GICC base address
        LDR     R0, [R1, #0xC]               ; R0: IAR

        PUSH    {R0, R1}

        CPS     #0x13                        ; Change to Supervisor mode to allow interrupt reentry

        PUSH    {LR}                         ; Save Supervisor LR
        LDR     R2, =SystemIrqHandler
        BLX     R2                           ; Call SystemIrqHandler with param IAR
        POP     {LR}

        CPS     #0x12                        ; Back to IRQ mode

        POP     {R0, R1}

        STR     R0, [R1, #0x10]              ; Now IRQ handler finished: write to EOIR

        POP     {R0}
        MSR     SPSR_CXSF, R0

        POP     {R0-R3, R12}
        POP     {LR}
        SUBS    PC, LR, #4

FIQ_Handler
        B .     ; Unexpected FIQ

        END
