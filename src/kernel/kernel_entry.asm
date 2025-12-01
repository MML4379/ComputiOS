[bits 64]

global kernel_entry
extern kernel_main

section .start

kernel_entry:
    ; jump straight to the kernel main function
    jmp kernel_main

.hang:
    cli
    hlt
    jmp .hang