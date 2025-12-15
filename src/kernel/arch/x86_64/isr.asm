bits 64
default rel

global idt_flush
global isr_stub_table

extern exception_dispatch
extern irq_dispatch

idt_flush:
    lidt [rdi]
    ret

%macro PUSH_REGS 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro POP_REGS 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

%macro ISR_NOERR 1
isr%1:
    PUSH_REGS
    push 0
    push %1
    mov rdi, rsp
    call exception_dispatch
    add rsp, 16
    POP_REGS
    iretq
%endmacro

%macro ISR_ERR 1
isr%1:
    PUSH_REGS
    push %1
    mov rdi, rsp
    call exception_dispatch
    add rsp, 8
    POP_REGS
    iretq
%endmacro

%assign i 0
%rep 32
    %if i == 8 || i == 10 || i == 11 || i == 12 || i == 13 || i == 14 || i == 17
        ISR_ERR i
    %else
        ISR_NOERR i
    %endif
    %assign i i+1
%endrep

%macro IRQ 1
isr%1:
    PUSH_REGS
    push 0
    push %1
    mov rdi, rsp
    call irq_dispatch
    add rsp, 16
    POP_REGS
    iretq
%endmacro

%assign i 32
%rep 16
    IRQ i
    %assign i i+1
%endrep

section .data
isr_stub_table:
%assign i 0
%rep 48
    dq isr%+i
    %assign i i+1
%endrep