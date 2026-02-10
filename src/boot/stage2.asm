[org 0x8000]

STAGE3_LOAD_ADDR equ 0x10000
STAGE3_SECTORS   equ 20
STAGE3_LBA_START equ 32

VBE_CTRL_PHYS    equ 0x8D000
VBE_MODE_PHYS    equ 0x8E000
FB_INFO_PHYS     equ 0x8F000

E820_BUFFER_SEG  equ 0x9000
E820_ENTRY_SIZE  equ 20

[bits 16]
start_stage2:
    cli
    mov [boot_drive], dl

    ; 1. Enable A20 Line
    call enable_a20

    ; 2. Get System Info (MemMap + VBE)
    call collect_e820
    call vbe_set_1280x720x32

    ; 3. Load Stage 3 (C Code) into Memory
    mov word [dap_num_sectors], STAGE3_SECTORS
    mov word [dap_buffer_offset], 0x0000
    mov word [dap_buffer_segment], 0x1000  ; 0x1000 * 16 = 0x10000
    mov dword [dap_lba_low], STAGE3_LBA_START
    mov dword [dap_lba_high], 0

    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_error

    ; 4. Switch to 32-bit Protected Mode
    lgdt [gdt_descriptor]
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode_entry

[bits 32]
protected_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000 ; Set stack below the kernel area

    ; Pass arguments to stage3
    ; void kboot_main(void* fb_info, void* mem_map, uint8_t boot_drive)
    
    ; Push args in reverse order (CDECL convention)
    movzx eax, byte [boot_drive]
    push eax            ; Arg3: boot drive
    push 0x9000         ; Arg2: MemMap (E820 buffer segment base)
    push 0x8F000        ; Arg1: FB Info (Physical address)

    ; Jump to stage3 (0x10000)
    call STAGE3_LOAD_ADDR
    
    ; If stage3 returns, hang
    cli
    hlt

disk_error:
    ; I will fill something in here eventually, just halt it for now
    hlt

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

vbe_set_1280x720x32:
    pusha
    push ds
    push es

    ; ES:DI -> VBE Controller Info block (must be in real mode memory)
    mov ax, VBE_CTRL_PHYS >> 4
    mov es, ax
    xor di, di

    ; Must put 'VBE2' signature to request VBE 2.0+ info
    mov dword [es:di], 0x32454256   ; 'VBE2'

    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    jne .fail

    ; ModeList pointer is at offset 0x0E (far ptr: seg:off)
    mov bx, [es:di + 0x0E]          ; offset
    mov cx, [es:di + 0x10]          ; segment

    mov ds, cx
    mov si, bx

.next_mode:
    lodsw                            ; AX = mode
    cmp ax, 0xFFFF
    je .fail

    push ax                          ; save mode

    ; Get ModeInfo into ES:DI at VBE_MODE_PHYS
    mov ax, VBE_MODE_PHYS >> 4
    mov es, ax
    xor di, di

    push bp
    mov bp, sp
    mov cx, [bp+2]                   ; CX = mode (from stack)
    pop bp
    mov ax, 0x4F01
    int 0x10
    cmp ax, 0x004F
    jne .pop_and_continue

    ; Check ModeAttributes: supported + graphics + LFB
    mov ax, [es:di + 0x00]
    test ax, 1                        ; bit0 supported
    jz .pop_and_continue
    test ax, (1 << 7)                 ; bit7 LFB available
    jz .pop_and_continue

    ; Check bpp at offset 0x19
    mov al, [es:di + 0x19]
    cmp al, 32
    jne .pop_and_continue

    ; Check memory model at 0x1B = 6 (Direct Color)
    mov al, [es:di + 0x1B]
    cmp al, 6
    jne .pop_and_continue

    ; Check resolution X (0x12), Y (0x14)
    mov ax, [es:di + 0x12]
    cmp ax, 1280
    jne .pop_and_continue

    mov ax, [es:di + 0x14]
    cmp ax, 720
    jne .pop_and_continue

    ; Found a match! Set mode with LFB bit (bit14 in BX)
    pop ax                            ; AX = mode
    mov bx, ax
    or  bx, 0x4000                    ; request linear framebuffer
    mov ax, 0x4F02
    int 0x10
    cmp ax, 0x004F
    jne .fail

    ; Write FramebufferInfo to FB_INFO_PHYS
    ; ModeInfo fields:
    ; BytesPerScanLine @ 0x10 (uint16)
    ; XRes @ 0x12 (uint16)
    ; YRes @ 0x14 (uint16)
    ; PhysBasePtr @ 0x28 (uint32)
    ; BPP @ 0x19 (uint8)

    mov ax, FB_INFO_PHYS >> 4
    mov ds, ax
    xor si, si                        ; DS:SI points to FB_INFO_PHYS

    ; phys_base (uint64) - store low dword, high dword = 0
    mov eax, [es:di + 0x28]
    mov [ds:si + 0], eax
    mov dword [ds:si + 4], 0

    ; width (uint32)
    movzx eax, word [es:di + 0x12]
    mov [ds:si + 8], eax

    ; height (uint32)
    movzx eax, word [es:di + 0x14]
    mov [ds:si + 12], eax

    ; pitch (uint32)
    movzx eax, word [es:di + 0x10]
    mov [ds:si + 16], eax

    ; bpp (uint32)
    movzx eax, byte [es:di + 0x19]
    mov [ds:si + 20], eax

    ; format (uint32)
    mov dword [ds:si + 24], 0

    jmp .ok

.pop_and_continue:
    pop ax
    jmp .next_mode

.fail:
    ; This should print something, but I'll do it later
.ok:
    pop es
    pop ds
    popa
    ret

collect_e820:
    ; ES = 0x9000
    mov ax, E820_BUFFER_SEG
    mov es, ax

    ; zero count at ES:0
    xor eax, eax
    mov [es:0], eax

    ; entries start at ES:0x0010 => phys 0x00090010
    mov di, 0x0010

    xor ebx, ebx                 ; continuation value = 0
    mov edx, 0x534D4150          ; 'SMAP'

.e820_next:
    mov eax, 0xE820
    mov ecx, E820_ENTRY_SIZE     ; max size we want
    ; ES:DI = buffer
    int 0x15
    jc .e820_done                ; CF=1 -> error or no more

    cmp eax, 0x534D4150          ; 'SMAP'?
    jne .e820_done

    cmp ecx, 20                  ; BIOS says how many bytes it actually wrote
    jb .skip_store               ; too small

    ; increment count
    mov eax, [es:0]
    inc eax
    mov [es:0], eax

    ; advance DI for next entry
    add di, E820_ENTRY_SIZE

.skip_store:
    test ebx, ebx
    jne .e820_next               ; EBX != 0 => more entries

.e820_done:
    ret

; GDT Entries:
; 0x00: null
; 0x08: 32-bit code
; 0x10: 32-bit data
; 0x18: 64-bit code

gdt_start:
    dq 0                      ; Null descriptor

gdt_code32:                   ; 32-bit code segment (selector 0x08)
    dw 0xFFFF                 ; Limit (0-15)
    dw 0x0000                 ; Base  (0-15)
    db 0x00                   ; Base  (16-23)
    db 10011010b              ; Access: present, ring 0, code, executable, readable
    db 11001111b              ; Flags: 4KB gran, 32-bit, limit high
    db 0x00                   ; Base  (24-31)

gdt_data:                     ; 32-bit data segment (selector 0x10)
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b              ; Access: present, ring 0, data, writable
    db 11001111b              ; Flags: 4KB gran, 32-bit
    db 0x00

gdt_code64:                   ; 64-bit code segment (selector 0x18)
    dw 0x0000                 ; Limit (ignored in long mode)
    dw 0x0000                 ; Base  (0-15)
    db 0x00                   ; Base  (16-23)
    db 10011010b              ; Access: present, ring 0, code
    db 00100000b              ; Flags: L=1 (64-bit), D=0, G=0
    db 0x00                   ; Base  (24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start