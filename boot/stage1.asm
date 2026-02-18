; ComputiOS Loader - Stage 1
; Lives in sector 0, simply jumps to stage 2

[bits 16]
[org 0x7C00]

STAGE2_LOAD_SEG equ 0x0000
STAGE2_LOAD_OFF equ 0x7E00
STAGE2_SECTORS equ 64 ; only need 32KiB

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl ; save this!

    ; print something
    mov si, msg_loading
    call print_string

    ; check for int 0x13
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc .disk_error ; no support
    cmp bx, 0xAA55 ; check signature
    jne .disk_error

    ; load stage 2
    mov word [dap_lba], 1
    mov word [dap_offset], STAGE2_LOAD_OFF
    mov cx, 4 ; loading in chunks of 16 sectors for compatibility

.load_loop:
    push cx
    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc .disk_error

    ; move to next chunk
    add word [dap_offset], 16 * 512 ; advance buffer by 16 sectors
    add word [dap_lba], 16 ; advance LBA by 16 sectors

    pop cx
    loop .load_loop

    ; jump to stage 2
    jmp 0x0000:STAGE2_LOAD_OFF

.disk_error:
    mov si, msg_disk_err
    call print_string
.halt:
    cli
    hlt
    jmp .halt

; uses 0x10 to print a string at ds:si
print_string:
    pusha
.print_loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .print_loop
.done
    popa
    ret

; data
boot_drive: db 0
msg_loading: db "ComputiOS - Stage 1", 0x0A, 0x0D, 0
msg_disk_err: db "Disk Error!", 0

; DAP (disk address packet)
align 4
dap:
    db 0x10 ; dap size=16
    db 0    ; reserved
    dw 16   ; sector count (16 per chunk)
dap_offset:
    dw STAGE2_LOAD_OFF ; offset
    dw STAGE2_LOAD_SEG ; segment
dap_lba:
    dd 1 ; LBA start
    dd 0 ; high 32 bits of LBA

; PARTITION TABLE

;   Disk Layout:
;       Sectors 0-2047: Bootloader and kernel
;       Sectors 2048+: FAT32 data partition

FAT32_PART_LBA equ 2048
FAT32_PART_SIZE equ 63448 ; 32MiB Image for now

times 446 - ($ - $$) db 0 ; pad to partition table

; Partition 1: FAT32 with LBA
db 0x00 ; status - not active
db 0x00, 0x21, 0x00 ; CHS start
db 0x0C ; Type - FAT32 with LBA
db 0x00, 0x00, 0x00 ; CHS end
dd FAT32_PART_LBA ; LBA of first sector
dd FAT32_PART_SIZE ; number of sectors

; partitions 2-4: empty
times 48 db 0

; boot signature
dw 0xAA55