[org 0x7c00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; save boot drive from BIOS
    mov [boot_drive], dl

    ; print message
    mov si, msg_stage1
    call print_string

    ; set up disk address packet
    mov word [dap_num_sectors], 64        ; load 64 sectors
    mov word [dap_buffer_offset], 0x8000  ; offset
    mov word [dap_buffer_segment], 0x0000 ; segment
    mov dword [dap_lba_low], 1            ; start at LBA 1 (immediately after bootsector)
    mov dword [dap_lba_high], 0

    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42                          ; extended read
    int 0x13
    jc disk_error

    ; jump to loaded stage2 at 0000:8000
    jmp 0x0000:0x8000

disk_error:
    mov si, msg_disk_error
    call print_string
hang:
    hlt
    jmp hang

; print_string: DS:SI -> teletype output until 0
print_string:
    pusha
.print_loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
    int 0x10
    jmp .print_loop
.done:
    popa
    ret

; Disk Address Packet (16 bytes)
disk_address_packet:
    db 16                      ; size
    db 0                       ; reserved
dap_num_sectors:
    dw 0
dap_buffer_offset:
    dw 0
dap_buffer_segment:
    dw 0
dap_lba_low:
    dd 0
dap_lba_high:
    dd 0

boot_drive: db 0

msg_stage1: db "Starting ComputiOS...", 0x0D, 0x0A, 0
msg_disk_error: db "Startup Error: Disk read error!", 0

times 510 - ($-$$) db 0
dw 0xAA55