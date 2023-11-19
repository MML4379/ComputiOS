; Step 1 - Set up CPU for hardware allocation
; Copyright (C) 2023 MML Tech LLC, All rights reserved.

section .data
    disk_isr dd 0          ; Placeholder for disk ISR address

section .bss
    read_buffer resb 512   ; Buffer for reading sectors

org 0x7c00
    jmp start

; Step 1 for switching to protected mode
enable_a20_line:
    cli                 ; Disable interrupts

    call wait_KB_ready  ; Wait for keyboard controller to be ready

    mov al, 0xd1        ; Write to keyboard controller
    out 0x64, al        ; Send command

    call wait_KB_ready  ; Wait for keyboard controller to be ready again

    mov al, 0xdf        ; Data to send to A20 bit
    out 0x60, al        ; Send the data

    sti                 ; Enable interrupts
    ret

; Step 2 for switching to protected mode
gdt_start:              ; Start label
    dd 0                ; Null descriptor

gdt_code:               ; Code segment descriptor
    dw 0xFFFF           ; Limit low
    dw 0                ; Base low
    db 0                ; Base middle
    db 10011010b        ; Access byte: 1 present, 00 ring, 1 code, 1 read/write
    db 11001111b        ; Granularity byte: 1 limit, 1 32-bit, 1 4KB granularity
    db 0                ; Base high

gdt_data:               ; Data segment descriptor
    dw 0xFFFF           ; Limit low (similar to code segment)
    dw 0                ; Base low
    db 0                ; Base middle
    db 10010010b        ; Access byte: 1 present, 00 ring, 1 data, 1 read/write
    db 11001111b        ; Granularity byte: 1 limit, 1 32-bit, 1 4KB granularity
    db 0                ; Base high

gdt_end:                ; End label

idt_start:              ; Start Label
    dw 0                ; IDT Limit
    dw 0                ; IDT base

idt_timer:              ; Timer interrupt descriptor
    dw 0                ; Offset low
    dw 0x8              ; Selector (code segment)
    db 0                ; Always 0
    db 0x8E             ; Type (32-bit interrupt gate), present

idt_keyboard:           ; Keyboard interrupt descriptor
    dw 0                ; Offset low
    dw 0x8              ; Selector (code segment)
    db 0                ; Always 0
    db 0x8E             ; Type (32-bit interrupt gate), present

idt_end:                ; End label

wait_KB_ready:
    in al, 0x64         ; Read status register
    test al, 2          ; Check bit 1 (input buffer status)
    jnz wait_KB_ready   ; If bit 1 is set, loop until clear
    ret

start:
    cli                 ; Disable interrupts

    ; Loads GDT and IDT
    lgdt [gdt_descriptor]
    lidt [idt_descriptor]

    ; Enable protected mode
    mov eax, cr0        ; Load control register CR0
    or eax, 0x1         ; Set protected mode bit
    mov cr0, eax        ; Update CR0 to enable protected mode

    jmp 0x8:loadStep2   ; Jump to Step 2 (See ComputiOS Boot Process)

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1    ; Limit
    dd gdt_start                  ; Base

; IDT descriptor
idt_descriptor:
    dw idt_end - idt_start - 1    ; Limit
    dd idt_start                  ; Base

loadStep2:
    ; Set up interrupt descriptor for disk interrupt
    mov eax, disk_isr                ; Address of disk ISR
    mov word [idt_timer], ax         ; Set the offset low
    mov word [idt_timer + 2], ax     ; Set the offset high

    ; Enable interrupts
    sti

    ; Issue 13h to read next sector
    mov ah, 0x02                     ; BIOS disk read function
    mov al, 2                        ; Number of sectors to read
    mov ch, 0                        ; Cylinder number
    mov cl, 2                        ; Sector number
    mov dh, 0                        ; Head number
    mov dl, 0x80                     ; Drive number (1st HDD)
    lea bx, [read_buffer]            ; Buffer address
    int 0x13                         ; BIOS interrupt

times 510 - ($-$$) db 0 ; pad to 510 bytes
dw 0xaa55               ; signature, needed by some BIOSes (2 bytes)