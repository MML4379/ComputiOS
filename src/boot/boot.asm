[BITS 16]
[ORG 0x7C00]
jmp start

%include 'src/boot/inc/print.asm'

start:
    cli                 ; Clear interrupts
    xor ax, ax          ; ax=0
    mov ds, ax          ; ds=0
    mov es, ax          ; es=0
    mov fs, ax          ; fs=0
    mov gs, ax          ; gs=0
    mov ss, ax          ; ss=0
    mov sp, 0x7C00      ; set stack pointer to boot location in memory

    ; Load message to memory
    mov si, startmsg    ; Load message to si register
    call print_string

    mov si, pmodemsg    ; Protected mode
    call print_string

    cli
    lgdt [gdtr]  ; load the gdt

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:PModeMain

[BITS 32]
PModeMain:
    xor ax, ax
    mov ax, 0x10         ; 0x10 is data segment selector
    mov ds, ax           ; ds=data segment
    mov es, ax           ; es=data segment
    mov fs, ax           ; fs=data segment
    mov gs, ax           ; gs=data segment
    mov ss, ax           ; ss=data segment
    mov esp, 0x600       ; new stack location, where our second stage loader is located

    mov esi, wpmode
    call sprint

    ; Hang the system
hang:
    jmp hang

gdt_start:
    dq 0x0000000000000000   ; null segment
    dq 0x00CF9A000000FFFF   ; code segment
    dq 0x00CF92000000FFFF   ; data segment
    dq 0x00CFFA000000FFFF   ; user code segment
    dq 0x00CFF2000000FFFF   ; user data segment
gdt_end:

gdtr:
    dw gdt_end - gdt_start - 1 ; Limit (size of GDT - 1)
    dd gdt_start               ; Base (address of GDT)

; Define the messages
startmsg db 'Starting ComputiOS...', 0x0A, 0x0D, 0
pmodemsg db 'Entering Protected Mode...', 0x0A, 0x0D, 0
wpmode   db 'Welcome to Protected Mode :)', 0

times 510-($-$$) db 0   ; Fill the rest of the boot sector with zeros
dw 0xAA55               ; Boot sector signature (required by some BIOSes)