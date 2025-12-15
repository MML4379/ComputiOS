bits 64
global gdt_flush
global tss_flush

gdt_flush:
    cli
    lgdt [rdi]

    mov ax, 0x10          ; kernel data
    mov ds, ax
    mov es, ax
    mov ss, ax

    xor eax, eax
    mov fs, ax
    mov gs, ax

    push 0x08             ; kernel code
    lea rax, [rel .reload]
    push rax
    retfq

.reload:
    sti
    ret

tss_flush:
    mov ax, 0x28          ; TSS selector
    ltr ax
    ret
