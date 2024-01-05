; bootsect.asm

BITS 16               ; Set the mode to 16-bit
ORG 0x7C00            ; Origin address where the bootloader will be loaded

jmp start             ; Jump to the start label

; Define constants
SECTOR_SIZE equ 512   ; Sector size in bytes

goto_pm:
    ; Load the second stage of the bootloader
    mov ax, 0x9000       ; Address where the second stage will be loaded
    mov es, ax
    mov bx, 0x0200       ; Offset where the second stage will be loaded
    mov ah, 0x02         ; Function number: Read sectors
    mov al, 1            ; Number of sectors to read
    mov ch, 0            ; Cylinder number
    mov dh, 0            ; Head number
    mov dl, 0x80         ; Drive number (e.g., 0x80 for the first hard disk)
    int 0x13             ; BIOS interrupt for disk I/O

    ; Check for error in reading the second stage
    jc boot_error

start:
    ; Set up the stack
    mov ax, 0x7C00
    add ax, SECTOR_SIZE
    mov ss, ax
    mov sp, 0           

    ; Check for CPUID support
    mov eax, 0
    cpuid
    test edx, 1 << 21   ; Check if the CPUID instruction is supported
    jz  no_cpuid_error

    ; Switch to protected mode
    call switch_to_protected_mode

    ; Jump to the second stage
    jmp 0x9000:0x0200

no_cpuid_error:
    ; Display an error message for CPUID not supported
    mov si, no_cpuid_msg
    call print_string

    ; Halt the system
    cli
    hlt

boot_error:
    ; Display an error message for disk read failure
    mov si, boot_error_msg
    call print_string

    ; Halt the system
    cli
    hlt

; Helper function to print a string
print_string:
    mov ah, 0x0E
    next_char:
        lodsb
        cmp al, 0
        je  done
        int 0x10
        jmp next_char
    done:
    ret

; Helper function to switch to protected mode
switch_to_protected_mode:
    cli                  ; Disable interrupts
    lgdt [gdt_descriptor] ; Load the Global Descriptor Table (GDT) descriptor
    mov eax, cr0
    or eax, 1 << 0       ; Set the PE (Protected Mode Enable) bit in CR0
    mov cr0, eax
    jmp CODE_SEG:goto_pm ; Jump to the next instruction in protected mode

; GDT (Global Descriptor Table) definition
gdt_start:
    dq 0                ; Null descriptor
    dq 0                ; Code segment descriptor (base = 0, limit = 4 GB, granularity = 4 KB, execute/read, present)
    dq 0                ; Data segment descriptor (base = 0, limit = 4 GB, granularity = 4 KB, read/write, present)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Size of the GDT - 1
    dd gdt_start             ; Address of the GDT

CODE_SEG equ gdt_start + 8 ; Offset of the code segment descriptor in the GDT

; Error message for CPUID not supported
no_cpuid_msg db "Error: CPUID instruction not supported", 0

; Error message for disk read failure
boot_error_msg db "Error loading second stage", 0

; Padding and magic number
times 510 - ($ - $$) db 0
dw 0xAA55