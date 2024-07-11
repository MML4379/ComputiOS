print_string:
    ; Print string pointed to by SI
    .print_next_char:
        lodsb                 ; Load next byte from string into AL
        or al, al             ; Check if end of string (0)
        jz .done              ; If zero, end of string
        call print_char       ; Print the character in AL
        jmp .print_next_char  ; Loop to print next character

    .done:
        ret

print_char:
    ; Print character in AL using BIOS interrupt
    mov ah, 0x0E            ; BIOS teletype function
    int 0x10                ; Call BIOS video interrupt
    ret

[BITS 32]
dochar:
    call cprint           ; print one character
sprint:
    mov eax, [esi]        ; string char to al
    lea esi, [esi+1]
    cmp al, 0
    jne dochar            ; done
    add byte [ypos], 1    ; down one row
    mov byte [xpos], 0    ; back to left
    ret

cprint:
    mov ah, 0x02          ; attributes = green on black
    mov ecx, eax          ; save char/attribute
    movzx eax, byte [ypos]
    mov edx, 160          ; 2 bytes (char/attribute)
    mul edx               ; for 80 columns
    movzx ebx, byte [xpos]
    shl ebx, 1            ; times 2 to skip attribute

    mov edi, 0xB8000      ; text vga memory
    add edi, eax          ; add y offset
    add edi, ebx          ; add x offset

    mov eax, ecx          ; restore char/attribute
    mov word [ds:edi], ax
    add byte [xpos], 1    ; advance to right

    ret

xpos db 0
ypos db 0

[BITS 16]