; Kernel entry point

[bits 64]

section .text.boot

extern cposkrnl
extern _bss_start
extern _bss_end

global _start
_start:
    ; rdi contains pointer to boot info struct
    ; save it
    push rdi

    ; clear bss section
    mov di, _bss_start
    mov rcx, _bss_end
    sub rcx, rdi
    shr rcx, 3 ; divide by 8
    xor rax, rax
    rep stosq

    ; restore BootInfo pointer
    mov rsp, kernel_stack_top
    xor rbp, rbp

    ; call kernel entry point
    call cposkrnl

    ; if kernel returns, halt CPU
.halt:
    cli
    hlt
    jmp .halt

; kernel stack
section .bss
align 16
kernel_stack_bottom:
    resb 16384
kernel_stack_top:
    