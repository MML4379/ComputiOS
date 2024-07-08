[BITS 16]
[ORG 0x7C00]

; Bootloader starts here
    cli                 ; Clear interrupts
    xor ax, ax          ; Zero out AX register
    mov ds, ax          ; Set DS to 0
    mov es, ax          ; Set ES to 0

    ; Load the message into the video memory
    mov si, message     ; Load address of message into SI
    call print_string   ; Call the print_string function

    ; Hang the system
hang:
    jmp hang            ; Infinite loop to hang the system

; Define the message
message db 'Hello, World!', 0

; Include print.asm for the print_string function
%include 'src/boot/inc/print.asm'

; Boot sector signature
times 510-($-$$) db 0   ; Fill the rest of the boot sector with zeros
dw 0xAA55               ; Boot sector signature