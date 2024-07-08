print_string:
    ; Print string pointed to by SI
    .print_next_char:
        lodsb               ; Load next byte from string into AL
        or al, al           ; Check if end of string (0)
        jz .done            ; If zero, end of string
        call print_char     ; Print the character in AL
        jmp .print_next_char; Loop to print next character

    .done:
        ret

print_char:
    ; Print character in AL using BIOS interrupt
    mov ah, 0x0E            ; BIOS teletype function
    int 0x10                ; Call BIOS video interrupt
    ret