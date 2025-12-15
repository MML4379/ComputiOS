global syscall_entry
extern syscall_dispatch

section .text
bits 64

syscall_entry:
    swapgs

    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov rdi, rax ; syscall number
    mov rsi, rbx ; arg1
    mov rdx, rcx ; arg2
    mov rcx, rdx ; arg3
    mov r8, r8 ; arg4
    mov r9, r9 ; arg5

    call syscall_dispatch

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    swapgs
    sysretq