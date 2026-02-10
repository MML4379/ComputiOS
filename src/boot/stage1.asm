[org 0x7c00]

; --- FAT32 Header Start ---
jmp short start
nop

; BIOS Parameter Block (BPB)
; We just reserve space here. When the disk gets formatted, these values end up being filled in.
; They can also be hardcoded, but I've chosen to leave them blank.
oem_name:           db "MSWIN4.1" ; 8 bytes
bytes_per_sector:   dw 512
sectors_per_cluster:db 1
reserved_sectors:   dw 32
num_fats:           db 2
root_dir_entries:   dw 0
total_sectors_16:   dw 0
media_type:         db 0xF8
sectors_per_fat_16: dw 0
sectors_per_track:  dw 32
num_heads:          dw 64
hidden_sectors:     dd 0
total_sectors_32:   dd 0
sectors_per_fat_32: dd 0
ext_flags:          dw 0
fs_version:         dw 0
root_cluster:       dd 2
fs_info:            dw 1
backup_boot_sector: dw 6
reserved_zeros:     times 12 db 0
drive_number:       db 0x80
reserved_1:         db 0
boot_signature:     db 0x29
volume_id:          dd 0x12345678
volume_label:       db "COMPUTIOS  " ; 11 bytes
fs_type:            db "FAT32   "    ; 8 bytes

; --- Boot Code Starts Here (Offset ~0x5A) ---
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; save boot drive from BIOS
    mov [boot_drive_storage], dl ; Don't overwrite the BPB 'drive_number' above, use a new var

    ; print message
    mov si, msg_stage1
    call print_string

    ; --- The "Reserved Sector" Trick ---
    ; FAT32 has "Reserved Sectors" before the actual file area starts.
    ; We configured 'reserved_sectors' = 32 above. 
    ; Sector 0 is this code. Sector 1 is FS Info. Sector 6 is Backup.
    ; That leaves Sectors 2-5 and 7-31 EMPTY.
    ; We can blindly load Stage 2 from there without parsing FAT!
    
    ; Setup Disk Address Packet to load Stage 2
    mov word [dap_num_sectors], 20       ; Load 20 sectors (10KB) - enough for Stage 2
    mov word [dap_buffer_offset], 0x8000 ; Target RAM
    mov word [dap_buffer_segment], 0x0000
    mov dword [dap_lba_low], 2           ; Start at Sector 2 (Skip Boot & FS Info)
    mov dword [dap_lba_high], 0

    mov si, disk_address_packet
    mov dl, [boot_drive_storage]
    mov ah, 0x42                         ; Extended Read
    int 0x13
    jc disk_error

    ; Jump to Stage 2
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

; Variables
boot_drive_storage: db 0

; Disk Address Packet (16 bytes)
disk_address_packet:
    db 16
    db 0
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

msg_stage1: db "Loading ComputiOS...", 0x0D, 0x0A, 0
msg_disk_error: db "Disk Err!", 0

times 510 - ($-$$) db 0
dw 0xAA55