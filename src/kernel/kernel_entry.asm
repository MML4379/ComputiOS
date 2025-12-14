[bits 64]

global kernel_entry
extern kernel_main

section .start

kernel_entry:
    ; ensure stack is 16-byte aligned before calling kernel
    ; it usually is, but don't assume that it is
    mov rbp, 0
    and rsp, -16
    sub rsp, 8 ; make it like a call frame (SysV alignment)

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang