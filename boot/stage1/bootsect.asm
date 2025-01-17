; -----------------------------------------------------
; | ComputiOS Stage 1 Loader                          |
; | This simply sets up FAT32 for reading the disk,   |
; | sets up 32-bit operation, and moves on to stage 2 |
; -----------------------------------------------------

[ORG 0x7C00]
[BITS 16]

%define LOAD_ADDR 0x1000
%define LOAD_SEG  (LOAD_ADDR >> 4)
%define LOAD_OFF  (LOAD_ADDR & 0xF)

jmp short _start
nop

%define OEMName           bp+0x03           ; Disk label
%define bytesPerSector    bp+0x0b           ; Bytes per sector
%define sectorsPerCluster bp+0x0d           ; Sectors per cluster
%define reservedSectors   bp+0x0e           ; Reserved sectors
%define fats              bp+0x10           ; Number of FATs
%define rootCluster       bp+0x2c           ; Cluster number of root directory
%define sectors           bp+0x13           ; Logical sectors
%define hugeSectors       bp+0x20           ; Total logical sectors
%define fatSectors32      bp+0x24           ; Sectors per FAT
%define extFlags          bp+0x28           ; Extended flags
%define fsVersion         bp+0x2a           ; File system version
%define fsInfo            bp+0x30           ; File system info sector
%define backupBootSector  bp+0x32           ; Backup boot sector location
%define driveNum          bp+0x40           ; Physical drive number
%define reserved          bp+0x41           ; Reserved
%define bootSignature     bp+0x42           ; Extended boot signature
%define volumeId          bp+0x43           ; Volume ID
%define volumeLabel       bp+0x47           ; Volume Label
%define fatTypeLabel      bp+0x52           ; File system type

times 0x3b db 0x00

;---------------------------------------------------
; Start of the main bootloader code and entry point
;---------------------------------------------------

global _start
_start:
    cli
    cld

    ; Initialize memory and stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Load GDT and enter protected mode
    call setup_protected_mode

    ; Enter 32-bit mode
[BITS 32]
%inc "src/boot/disk.asm"
protected_mode_entry:
    mov ax, 0x10                           ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000                        ; Set stack pointer

    ; Relocated entry point for FAT32 operations
    call relocated_entry

;---------------------------------------------------
; Setup protected mode
;---------------------------------------------------

setup_protected_mode:
    cli                                    ; Disable interrupts
    lgdt [gdt_descriptor]                 ; Load GDT

    mov eax, cr0
    or eax, 1                             ; Set PE bit in CR0
    mov cr0, eax
    jmp 0x08:protected_mode_entry          ; Far jump to flush prefetch queue

;---------------------------------------------------
; Relocated boot sector entry point
;---------------------------------------------------

relocated_entry:
    ; Read FAT32-specific fields from BPB
    mov eax, dword [rootCluster]
    mov [rootDirCluster], eax

    ; Proceed to Stage 2
    jmp LOAD_ADDR

;---------------------------------------------------
; Bootloader variables and GDT
;---------------------------------------------------

gdt:
    dw 0x0000, 0x0000, 0x0000, 0x0000      ; Null descriptor
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF      ; Code segment
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF      ; Data segment

gdt_descriptor:
    dw gdt - 1                             ; Limit
    dd gdt                                 ; Base

rootDirCluster  dd 0                       ; Root directory starting cluster

times 510 - ($ - $$) db 0                  ; Pad boot sector
dw 0xaa55                                  ; Boot signature
