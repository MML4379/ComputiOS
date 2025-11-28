bits 64

global idt_flush
global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

global isr32, isr33, isr34, isr35, isr36, isr37, isr38, isr39
global isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47

extern exception_handler
extern irq_handler

idt_flush:
    mov rax, rdi
    lidt [rax]
    ret

; exception stubs

%macro MAKE_EXC 1
isr%1:
    mov rdi, %1
    call exception_handler
.hang%1:
    hlt
    jmp .hang%1
%endmacro

MAKE_EXC 0
MAKE_EXC 1
MAKE_EXC 2
MAKE_EXC 3
MAKE_EXC 4
MAKE_EXC 5
MAKE_EXC 6
MAKE_EXC 7
MAKE_EXC 8
MAKE_EXC 9
MAKE_EXC 10
MAKE_EXC 11
MAKE_EXC 12
MAKE_EXC 13
MAKE_EXC 14
MAKE_EXC 15
MAKE_EXC 16
MAKE_EXC 17
MAKE_EXC 18
MAKE_EXC 19
MAKE_EXC 20
MAKE_EXC 21
MAKE_EXC 22
MAKE_EXC 23
MAKE_EXC 24
MAKE_EXC 25
MAKE_EXC 26
MAKE_EXC 27
MAKE_EXC 28
MAKE_EXC 29
MAKE_EXC 30
MAKE_EXC 31

; irq stubs

%macro PUSH_ALL 0
    push rax
    push rcx
    push rdx
    push rbx
    push rsp
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

%macro POP_ALL 0
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
    pop rsp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

%macro MAKE_IRQ 1
isr%1:
    PUSH_ALL
    mov rdi, %1
    call irq_handler
    POP_ALL
    iretq
%endmacro

MAKE_IRQ 32
MAKE_IRQ 33
MAKE_IRQ 34
MAKE_IRQ 35
MAKE_IRQ 36
MAKE_IRQ 37
MAKE_IRQ 38
MAKE_IRQ 39
MAKE_IRQ 40
MAKE_IRQ 41
MAKE_IRQ 42
MAKE_IRQ 43
MAKE_IRQ 44
MAKE_IRQ 45
MAKE_IRQ 46
MAKE_IRQ 47