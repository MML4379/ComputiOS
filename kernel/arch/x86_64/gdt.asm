; GDT and TSS assembly stubs

[bits 64]

section .text

; void gdt_flush(uint64_t gdt_ptr_addr), rdi contains gdt_ptr_addr
global gdt_flush
gdt_flush:
    lgdt [rdi]
    ; reload cs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    
    push 0x08
    lea rax, [rel .reload_cs]
    push rax
    retfq
.reload_cs:
    ret

; void tss_flush(uint16_t selector), rdi contains selector
global tss_flush
tss_flush:
    mov ax, di
    ltr ax
    ret