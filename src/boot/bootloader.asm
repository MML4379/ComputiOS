section .text
    ; Bootloader entry point
    global _start

    ; Constants
    KERNEL_LOAD_ADDRESS equ 0x1000  ; Address where the kernel will be loaded
    KERNEL_ENTRY_POINT  equ 0x20000 ; Address where the kernel entry point will be set

    ; Function to print a null-terminated string to the screen
    print_string:
        ; Arguments:
        ;   rdi - address of the null-terminated string

    .print_loop:
        ; Load the current character
        mov al, [rdi]

        ; Check for the null terminator
        cmp al, 0
        je .print_done

        ; Print the character
        call print_char

        ; Move to the next character in the string
        inc rdi

        ; Continue the loop
        jmp .print_loop

    .print_done:
        ret

    _start:
        ; Set up segments
        xor ax, ax             ; Clear ax register
        mov ds, ax             ; Set data segment to 0
        mov es, ax             ; Set extra segment to 0
        mov fs, ax             ; Set fs segment to 0
        mov gs, ax             ; Set gs segment to 0

        ; Switch to 64-bit mode
        cli                    ; Clear interrupts

        mov eax, cr0           ; Get the current value of control register CR0
        or eax, 0x1            ; Set the PE (Protection Enable) bit to switch to protected mode
        mov cr0, eax           ; Update CR0

         ; Print a welcome message
        mov rdi, welcome_message
        call print_string

        ; Print loading message
        mov rdi, loading_message
        call print_string

        ; Load kernel into memory
        mov ah, 0x02           ; BIOS function: read sectors
        mov al, 1              ; Number of sectors to read
        mov ch, 0              ; Cylinder number
        mov dh, 0              ; Head number
        mov dl, 0x80           ; Drive number (0x80 for the first hard drive)
        mov bx, KERNEL_LOAD_ADDRESS ; Destination address to load the kernel
        mov es, bx             ; Set es to the segment of the destination address
        mov bx, 0              ; Offset within the segment

        int 0x13               ; BIOS interrupt for disk I/O

        ; Check for errors
        jc _error               ; Jump to _error if the carry flag is set

        

        jmp CODE_SEG:64bit_mode ; Jump to the 64-bit mode

    64bit_mode:
        mov ax, DATA_SEG       ; Set data segment selector
        mov ds, ax             ; Load the data segment selector into the data segment register

        mov ax, CODE_SEG       ; Set code segment selector
        mov cs, ax             ; Load the code segment selector into the code segment register

        mov rsp, KERNEL_LOAD_ADDRESS + 0x8000 ; Set stack pointer to a suitable location in the loaded kernel

        ; Call the kernel entry point
        call KERNEL_ENTRY_POINT

    _error:
        ; Handle disk read error (you can add your error handling code here)
        mov rdi, disk_read_error
        call print_string

        ; Infinite loop
        cli                    ; Disable interrupts
        hlt                    ; Halt the system

    ; Padding to make sure the bootloader is 512 bytes (one sector)
    times 510-($-$$) db 0
    dw 0xAA55                ; Boot signature

CODE_SEG equ 0x8 ; Code segment selector in GDT
DATA_SEG equ 0x10 ; Data segment selector in GDT
loading_message db "Loading CPOSKRNL", 0xD, 0xA, 0
welcome_message db "ComputiOS v0.01.0", 0xD, 0xA, 0
disk_read_error db "Disk read error!", 0xD, 0xA, 0

; Function to print a character to the screen
print_char:
    ; Arguments:
    ;   dl - character to print

    ; Video memory address
    mov edi, 0xB8000

    ; Calculate the offset
    mov eax, edi
    shl eax, 6
    add eax, edi

    ; Move the offset to edi
    mov edi, eax

    ; Calculate the position on the screen
    shl edx, 1
    add edi, edx

    ; Store the character and attribute in the video memory
    mov byte [edi], dl ; Character
    mov byte [edi + 1], 0x0F ; Attribute (white on black)

    ret