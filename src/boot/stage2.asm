; I never said I was good at assembly, but hey at least it works

[org 0x8000]

KERNEL_LBA_START equ 33           ; where Makefile dd's kernel.bin
KERNEL_SECTOR_COUNT equ 128          ; 128 * 512 = 64 KiB max kernel size

KERNEL_LOAD_REAL equ 0x00010000   ; where BIOS loads kernel (real mode)
KERNEL_PHYS equ 0x00100000   ; final kernel location (identity-mapped)

KERNEL_DWORD_COUNT equ (KERNEL_SECTOR_COUNT * 512) / 4   ; for rep movsd

E820_BUFFER_SEG equ 0x9000
E820_ENTRY_SIZE equ 20

FB_INFO_PHYS equ 0x0008F000
VBE_CTRL_PHYS equ 0x0008D000
VBE_MODE_PHYS equ 0x0008E000

[bits 16]
start_stage2:
    cli

    ; Preserve boot drive from BIOS
    mov [boot_drive], dl

    ; Print startup msg
    mov si, msg_stage2_real
    call print_string_16

    mov word [dap_num_sectors], KERNEL_SECTOR_COUNT
    mov word [dap_buffer_offset], 0x0000
    mov word [dap_buffer_segment], 0x1000 
    mov dword [dap_lba_low], KERNEL_LBA_START
    mov dword [dap_lba_high], 0

    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc kernel_disk_error

    call vbe_set_1280x720x32

    call collect_e820

    lgdt [gdt_descriptor]

    mov eax, cr0
    or  eax, 1          ; set PE bit
    mov cr0, eax

    ; Far jump into 32-bit code segment
    jmp 0x08:protected_mode_entry

; set VBE mode
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
    ; If VBE fails, you can keep text mode and still boot.
    ; For debugging, you might want to print something, but keep it simple.
.ok:
    pop es
    pop ds
    popa
    ret

; 16-bit print routine
print_string_16:
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

kernel_disk_error:
    mov si, msg_kernel_disk_error
    call print_string_16
.hang_err:
    hlt
    jmp .hang_err

; disk address packet for int 0x13
disk_address_packet:
    db 16                ; size
    db 0                 ; reserved
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


msg_stage2_real:        db "Doing some prep...", 0x0D, 0x0A, 0
msg_kernel_disk_error:  db "ERROR: Couldn't find the kernel!", 0


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

[bits 32]
protected_mode_entry:
    ; Load data segments (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up a simple stack
    mov esp, 0x9FC00

    ; DEBUG breadcrumb: write "PM" at top-left
    mov dword [0xB8000], 0x2F4D2F50

    ; copy kernel to memory
    mov esi, KERNEL_LOAD_REAL
    mov edi, KERNEL_PHYS
    mov ecx, KERNEL_DWORD_COUNT
    rep movsd

    ; setup static page tables
    ; Enable PAE (CR4.PAE = 1)
    mov eax, cr4
    or  eax, 1 << 5
    mov cr4, eax

    ; Load PML4 base into CR3
    mov eax, p4_table
    mov cr3, eax

    ; Enable Long Mode (LME) via IA32_EFER (MSR 0xC0000080)
    mov ecx, 0xC0000080         ; IA32_EFER
    rdmsr
    or  eax, 1 << 8             ; set LME (bit 8)
    wrmsr

    ; Enable paging
    mov eax, cr0
    or  eax, 1 << 31
    mov cr0, eax

    ; Far jump to 64-bit code segment
    jmp 0x18:long_mode_entry

[bits 64]
extern kernel_main

long_mode_entry:
    ; Load data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; Set up a 64-bit stack
    mov rsp, 0x9F000

    ; quick sanity message before going to kernel
    mov rdi, 0xB8000
    mov rsi, msg_long_mode
    mov bl, 0x0F

.print_loop_64:
    lodsb
    test al, al
    jz .after_msg
    mov [rdi], al
    mov [rdi + 1], bl
    add rdi, 2
    jmp .print_loop_64

.after_msg:
    mov rdi, FB_INFO_PHYS
    mov rax, KERNEL_PHYS
    jmp rax

msg_long_mode: db "CPU supports 64-bit, switching to kernel...", 0

; page tables
align 4096
p4_table:
    dq p3_table + 0x03          ; present + writable
    times 511 dq 0

align 4096
p3_table:
    dq p2_table + 0x03          ; present + writable
    times 511 dq 0

align 4096
p2_table:
    ; Identity-map the first 32MiB (2MiB pages * 16 entries). This
    ; ensures the kernel can access physical memory up to 32MiB while
    ; it's still running on the initial identity-mapped page tables.
    ; Flags: P=1, RW=1, PS=1 -> 0x83
    dq 0x0000000000000083       ; 2MiB page at 0x00000000
    dq 0x0000000000200083       ; 2MiB page at 0x00200000
    dq 0x0000000000400083       ; 2MiB page at 0x00400000
    dq 0x0000000000600083       ; 2MiB page at 0x00600000
    dq 0x0000000000800083       ; 2MiB page at 0x00800000
    dq 0x0000000000A00083       ; 2MiB page at 0x00A00000
    dq 0x0000000000C00083       ; 2MiB page at 0x00C00000
    dq 0x0000000000E00083       ; 2MiB page at 0x00E00000
    dq 0x0000000001000083       ; 2MiB page at 0x01000000
    dq 0x0000000001200083       ; 2MiB page at 0x01200000
    dq 0x0000000001400083       ; 2MiB page at 0x01400000
    dq 0x0000000001600083       ; 2MiB page at 0x01600000
    dq 0x0000000001800083       ; 2MiB page at 0x01800000
    dq 0x0000000001A00083       ; 2MiB page at 0x01A00000
    dq 0x0000000001C00083       ; 2MiB page at 0x01C00000
    dq 0x0000000001E00083       ; 2MiB page at 0x01E00000
    times 495 dq 0