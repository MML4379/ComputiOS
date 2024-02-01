section .text
    global _start

_start:
    ; Set up stack for the bootloader
    mov rsp, BOOT_STACK_TOP

    ; Call the C++ function
    call kernel_entry

    ; If kernel_basic returns, enter an infinite loop
hang:
    jmp hang

section .bss
    ; Define a stack for the bootloader
    BOOT_STACK_SIZE equ 0x1000 ; 4 KB stack size
    BOOT_STACK_TOP equ BOOT_LOAD_ADDRESS + BOOT_STACK_SIZE


section .text
    ; External declaration for kernel_basic function
    extern kernel_basic

    ; Entry point for C++ kernel function
    global kernel_entry

kernel_entry:
    ; Set up stack for the C++ code
    mov rsp, KERNEL_STACK_TOP

    ; Call the C++ function
    call kernel_basic

    jmp hang