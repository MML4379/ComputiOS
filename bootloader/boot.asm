[BITS 16]
[ORG 0x7C00]

start:
    cli                 ; Clear interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; Set stack pointer
    mov bp, sp

    ; Enable A20 line
.enable_a20:
    in al, 0x64
    test al, 2
    jnz .enable_a20
    mov al, 0xD1
    out 0x64, al
    .wait_a20_1:
        in al, 0x64
        test al, 2
        jnz .wait_a20_1
    mov al, 0xDF
    out 0x60, al
    .wait_a20_2:
        in al, 0x64
        test al, 2
        jnz .wait_a20_2

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush the pipeline and update CS
    jmp 0x08:protected_mode

[BITS 32]
protected_mode:
    ; Set up data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; Set up page tables for long mode
    ; PML4, PDPT, PDT, PT need to be set up properly in loader.c
    mov eax, page_directory_pointer_table
    mov cr3, eax

    ; Enable long mode
    mov ecx, 0xC0000080      ; Address of the Extended Feature Enable Register (EFER)
    rdmsr
    or eax, 0x100            ; Set the LME (Long Mode Enable) bit
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 0x80000000       ; Set the PG (Paging) bit
    mov cr0, eax

    ; Far jump to flush the pipeline and update CS in long mode
    jmp 0x28:long_mode

[BITS 64]
long_mode:
    ; Set up data segments for long mode
    mov ax, 0x30
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Call the second stage bootloader (loader.c)
    call loader_main

gdt_start:
    ; Null descriptor
    dq 0x0

    ; 32-bit code segment descriptor
    dq 0x00CF9A000000FFFF

    ; 32-bit data segment descriptor
    dq 0x00CF92000000FFFF

    ; 64-bit code segment descriptor
    dq 0x00AF9A000000FFFF

    ; 64-bit data segment descriptor
    dq 0x00AF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dq gdt_start

section .bss
align 0x1000
page_directory_pointer_table: resq 1
page_directory: resq 1
page_table: resq 512

times 510-($-$$) db 0
dw 0xAA55