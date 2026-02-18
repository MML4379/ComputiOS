; ISR and IRQ stubs

[bits 64]

section .text

extern isr_handler

; we will need to push our own error code since these ISRs don't do it themselves
%macro ISR_NOERR 1
global isr%1
isr%1:
    push qword, 0 ; dummy error code
    push qword, %1 ; interrupt number
    jmp isr_common_stub
%endmacro

; just push the interrupt number, error code is already on stack
%macro ISR_ERR 1
global isr%1
isr%1:
    push qword, %1 ; interrupt number
    jmp isr_common_stub
%endmacro

%macro IRQ 2
global irq%1
irq%1:
    push qword 0 ; dummy error code
    push qword %2 ; vector number
    jmp isr_common_stub
%endmacro

; CPU Exceptions
ISR_NOERR 0 ; #DE Divide Error
ISR_NOERR 1 ; #DB Debug
ISR_NOERR 2 ; NMI
ISR_NOERR 3 ; #BP Breakpoint
ISR_NOERR 4 ; #OF Overflow
ISR_NOERR 5 ; #BR Bound Range
ISR_NOERR 6 ; #UD Invalid Opcode
ISR_NOERR 7 ; #NM Device Not Available
ISR_ERR 8 ; #DF Double Fault
ISR_NOERR 9 ; Coprocessor Segment Overrun
ISR_ERR 10 ; #TS Invalid TSS
ISR_ERR 11 ; #NP Segment Not Present
ISR_ERR 12 ; #SS Stack-Segment Fault
ISR_ERR 13 ; #GP General Protection Fault
ISR_ERR 14 ; #PF Page Fault
ISR_NOERR 15 ; Reserved
ISR_NOERR 16 ; #MF x87 FP Exception
ISR_ERR 17 ; #AC Alignment Check
ISR_NOERR 18 ; #MC Machine Check
ISR_NOERR 19 ; #XM SIMD FP Exception
ISR_NOERR 20 ; #VE Virtualization
ISR_ERR 21 ; #CP Control Protection
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; Hardware IRQs
IRQ 0, 32 ; PIT Timer
IRQ 1, 33 ; Keyboard
IRQ 2, 34 ; Cascade
IRQ 3, 35 ; Serial Port 2
IRQ 4, 36 ; Serial Port 1
IRQ 5, 37 ; LPT2
IRQ 6, 38 ; Floppy
IRQ 7, 39 ; LPT1 / Spurious
IRQ 8, 40 ; CMOS RTC
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44 ; PS/2 Mouse
IRQ 13, 45 ; FPU
IRQ 14, 46 ; Master ATA
IRQ 15, 47 ; Slave ATA

; Syscall
ISR_NOERR 128

isr_common_stub:
    ; Save general-purpose registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Pass pointer to InterruptFrame as first argument
    mov rdi, rsp
    cld
    call isr_handler

    ; Restore general-purpose registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; Remove int_no and err_code from stack
    add rsp, 16

    iretq

global idt_flush
idt_flush:
    lidt [rdi]
    ret
