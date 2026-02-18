; ComputiOS Loader - Stage 2
; Must query for E820 map, set video mode, build the BootInfo structure, enter 32-bit mode, set up 4 level paging, enter long mode,
; and jump to the kernel!

[bits 16]
[org 0x7E00]

; CONSTANTS
KERNEL_PHYS equ 0x100000
KERNEL_VIRT_HIGH equ 0xFFFFFFFF80100000
BOOTINFO_ADDR equ 0x9000
E820_MAP_ADDR equ 0x9100
E820_MAX_ENTRIES equ 128
PAGE_TABLES_ADDR equ 0x200000
STACK64_TOP equ 0x90000

; BootInfo struct offsets
BI_MAGIC equ 0
BI_FB_ADDR equ 8
BI_FB_WIDTH equ 16
BI_FB_HEIGHT equ 20
BI_FB_PITCH equ 24
BI_FB_BPP equ 28
BI_MMAP_COUNT equ 32
BI_MMAP_ADDR equ 40
BI_KERNEL_PHYS equ 48
BI_KERNEL_SIZE equ 56
BI_ACPI_RSDP equ 64

BOOTINFO_MAGIC equ 0x43504F53424F4F54 ; 'CPOSBOOT'

stage2_entry:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive_s2], dl

    mov si, s2_banner
    call print16

    call query_e820
    call setup_vesa
    call load_kernel
    call build_bootinfo

    mov si, s2_msg_pm
    call print16

    cli
    lgdt [gdt32_ptr]

    ; enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; far jump to flush pipeline and use 32-bit code segment
    jmp 0x08:pm_entry

; Real Mode Subroutines
print16:
    pusha
.p16loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .p16loop
.done:
    popa
    ret

query_e820:
    pusha
    mov si, s2_msg_e820
    call print16

    xor ebx, ebx ; continuation value (0 = start)
    mov di, E820_MAP_ADDR
    xor bp, bp ; entry counter

.e820_loop:
    mov eax, 0xE820
    mov ecx, 24 ; buffer size
    mov edx, 0x534D4150 ; 'SMAP' magic
    int 0x15
    jc .e820_done ; CF set - an error occured or it finished

    cmp eax, 0x534D4150
    jne .e820_done

    ; valid entry
    inc bp
    add di, 24

    cmp bp, E820_MAX_ENTRIES
    jge .e820_done

    or ebx, ebx ; ebx=0 is last entry
    jnz .e820_loop

.e820_done:
    mov [e820_count], bp
    popa
    ret

; this should work on VMs and real hardware
setup_vesa:
    pusha
    mov si, s2_msg_vesa
    call print16

    ; get VBE controller info
    mov di, vbe_info_block
    mov byte [di+0], 'V'
    mov byte [di+1], 'B'
    mov byte [di+2], 'E'
    mov byte [di+3], '2'

    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    jne .vesa_fail

    ; get pointer to mode list
    ; far pointer at offset 0x0E
    mov si, [vbe_info_block + 0x0E] ; offset of mode list
    mov ax, [vbe_info_block + 0x10] ; segment of mode list
    mov [.mode_seg], ax
    mov [.mode_off], si

    ; try each preferred resolution against the full mode list
    mov bp, preferred_res
.try_resolution:
    cmp word [bp], 0 ; end of preferred list?
    je .vesa_fail

    ; load desired width, height, bpp
    mov ax, [bp]
    mov [.want_w], ax
    mov ax, [bp+2]
    mov [.want_h], ax
    mov al, [bp+4]
    mov [.want_bpp], al

    ; print a dot to indicate progress
    push ds
    mov ah, 0x0E
    mov al, 0xB7 ; middle dot in ASCII code
    xor bh, bh
    int 0x10
    pop bp

    ; walk the mode list for this resolution
    push ds
    mov ax, [.mode_seg]
    mov ds, ax
    mov si, [cs:.mode_off]

.scan_modes:
    lodsw ; load mode number
    cmp ax, 0xFFFF ; end of mode list?
    je .next_resolution

    ; save mode number and mode list position
    mov [cs.cur_mode], ax
    push si
    push ds

    ; restore ds=0 for VBE call
    xor bx, bx
    mov ds, bx

    ; get mode info
    mov cx, [.cur_mode]
    push cx
    moc ax, 0x4F01
    mov di, vbe_mode_info
    int 0x10
    pop cx
    cmp ax, 0x004F
    jne .scan_next

    ; check mode attributes: supported + graphics + LFB
    mov ax, [vbe_info_block]
    and ax, 0x0091 ; bits 0, 4, 7
    cmp ax, 0x0091
    jne .scan_next

    ; check resolution matches
    mov ax, [vbe_mode_info + 18] ; x-res
    cmp ax, [.want_w]
    jne .scan_next
    mov ax, [vbe_info_block + 20] ; y-res
    cmp ax, [.want_h]
    jne .scan_next

    ; check bpp matches
    mov al, [vbe_mode_info + 25] ; bpp
    cmp al, [.want_bpp]
    jne .scan_next

    ; found matching mode!
    or cx, 0x4000 ; request LFB
    mov bx, cx
    mov ax, 0x4F02
    int 0x10
    cmp ax, 0x004F
    jne .scan_next

    ; store fb info
    mov eax, [vbe_mode_info + 40] ; physical base pointer
    mov [vesa_fb_addr], eax

    movzx eax, word [vbe_mode_info + 18] ; x-res
    mov [vesa_fb_width], eax

    movzx eax, word [vbe_mode_info + 20]   ; y-res
    mov [vesa_fb_height], eax

    movzx eax, word [vbe_mode_info + 16]   ; bytes per scan line
    mov [vesa_fb_pitch], eax

    movzx eax, byte [vbe_mode_info + 25]   ; bpp
    mov [vesa_fb_bpp], eax

    pop ds
    pop si
    pop ds ; restore ds
    popa
    ret

.scan_next:
    pop ds ; restore mode list segment
    pop si ; restore mode list position
    jmp .scan_modes

.next_resolution:
    pop ds ; restore ds
    add bp, 5 ; next preferred resolution entry
    jmp .try_resolution

; local variables
.mode_seg dw 0
.mode_off dw 0
.cur_mode dw 0
.want_w dw 0
.want_h dw 0
.want_bpp db 0

.vesa_fail:
    mov si, s2_msg_vesa_fail
    call print16
    cli
    hlt

load_kernel:
    pusha
    mov si, s2_msg_kernel
    call print16

    ; enable a20 gate
    in al, 0x92
    or al, 2
    and al, 0xFE
    out 0x92, al

    ; enter unreal mode
    cli
    push ds

    lgdt [gdt32_ptr]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    mov bx, 0x10 ; data segment - 32-bit, 4GiB limit
    mov ds, bx
    or al, 0xFE
    mov cr0, eax ; back to real mode

    pop ds
    sti

    ; load kernel in chunks to a buffer, then copy to higgh memory using 32-bit addressing (why we entered unreal mode)
    mov dword [kernel_lba], 64 ; kernel at sectors immediately after stage2
    mov dword [kernel_dest], KERNEL_PHYS
    mov cx, 8 ; 8 chunks * 32KiB = 256KiB kernel max size - this obviously will be increased later but it works for now

.load_loop:
    push cx

    ; read 64 sectors to temp buffer at 0x10000
    mov word [dap_s2 + 2], 64
    mov word [dap_s2 + 4], 0x0000 ; offset
    mov word [dap_s2 + 6], 0x1000 ; segment (0x1000:0x0000 = 0x10000)
    mov eax, [kernel_lba]
    mov [dap_s2 + 8], eax
    mov dword [dap_s2 + 12], 0

    mov si, dap_s2
    mov ah, 0x42
    mov dl, [boot_drive_s2]
    int 0x13
    jc .load_done ; TODO: Add error checking

    ; copy 32KiB from 0x10000 to kernel_dest using 32-bit addressing
    mov esi, 0x10000
    mov edi, [kernel_dest]
    mov ecx, 8192 ; 32768 / 4 = 8192 dwords
    a32 rep movsd

    add dword [kernel_dest], 0x8000 ; +32KiB
    add dword [kernel_lba], 64

    pop cx
    loop .load_loop
    jmp .load_finish

.load_done:
    pop cx
.load_finish:
    ; store kernel size
    mov eax, [kernel_dest]
    sub eax, KERNEL_PHYS
    mov [kernel_size], eax

    popa
    ret

built_bootinfo:
    pusha
    mov di, BOOTINFO_ADDR

    ; magic (8 bytes)
    mov dword [di + BI_MAGIC], 0x424F44F54 ; "BOOT" (low)
    mov dword [di + BI_MAGIC + 4], 0x43504F53 ; "CPOS" (high)

    ; framebuffer address (8 bytes)
    mov eax, [vesa_fb_addr]
    mov [di + BI_FB_ADDR], eax
    mov dword [di + BI_FB_ADDR + 4], 0

    ; framebuffer dimensions
    mov eax, [vesa_fb_width]
    mov [di + BI_FB_WIDTH], eax

    mov eax, [vesa_fb_height]
    mov [di + BI_FB_HEIGHT], eax

    mov eax, [vesa_fb_pitch]
    mov [di + BI_FB_PITCH], eax

    mov eax, [vesa_fb_bpp]
    mov [di + BI_FB_BPP], eax

    ; Memory map
    movzx eax, word [e820_count]
    mov [di + BI_MMAP_COUNT], eax
    mov dword [di + BI_MMAP_COUNT + 4], 0

    mov dword [di + BI_MMAP_ADDR], E820_MAP_ADDR
    mov dword [di + BI_MMAP_ADDR + 4], 0

    ; Kernel physical
    mov dword [di + BI_KERNEL_PHYS], KERNEL_PHYS
    mov dword [di + BI_KERNEL_PHYS + 4], 0

    mov eax, [kernel_size]
    mov [di + BI_KERNEL_SIZE], eax
    mov dword [di + BI_KERNEL_SIZE + 4], 0

    popa
    ret

; 16-bit data
boot_drive_s2: db 0
e820_count: dw 0
kernel_lba: dd 0
kernel_dest: dd 0
kernel_size: dd 0

vesa_fb_addr: dd 0
vesa_fb_width: dd 0
vesa_fb_height: dd 0
vesa_fb_pitch: dd 0
vesa_fb_bpp: dd 0

; I have this only set to 16:9 resolutions, but this will be cleaned up in the future to be far more flexible
; (below 720p it goes to 4:3 resolutions for a safety net)
preferred_res:
    dw 1920, 1080
    db 32           ; 1080p 32bpp
    
    dw 1440, 900
    db 32           ; 900p 32bpp

    dw 1280, 720
    db 32           ; 720p 32bpp

    dw 800, 600
    dw 32           ; 600p 32bpp

    dw 640, 480
    dw 32           ; 480p 32bpp

    dw 1920, 1080
    db 24           ; 1080p 24bpp
    
    dw 1440, 900
    db 24           ; 900p 24bpp

    dw 1280, 720
    db 24           ; 720p 24bpp

    dw 800, 600
    dw 24           ; 600p 24bpp

    dw 640, 480
    dw 24           ; 480p 24bpp

; VBE info blocks
align 16
vbe_info_block: times 512 db 0
vbe_mode_info: times 256 db 0

; 32-bit GDT
align 16
gdt32_start:
    ; null descriptor
    dq 0
    ; code segment
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00
    ; data segment
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00
gdt32_end:

gdt32_ptr:
    dw gdt32_end - gdt32_start - 1
    dd gdt32_start

; protected mode entry
[bits 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x90000

    ; set up 4-level paging
    ; identity map first 4GiB and map higher half using 2MiB huge pages

    ; clear page table area
    mov edi, PAGE_TABLES_ADDR
    xor eax, eax
    mov ecx, 0x4000 ; 4096 dwords
    rep stosd

    ; PML4 (Page Map Level 4) at PAGE_TABLES_ADDR + 0x0000
    ; Entry 0 -> PDPT_low   (identity map)
    ; Entry 511 -> PDPT_high (higher half: 0xFFFFFFFF80000000)
    mov edi, PAGE_TABLES_ADDR
    mov eax, PAGE_TABLES_ADDR + 0x1000 ; PDPT_low
    or eax, 0x03 ; Present + Writable
    mov [edi], eax

    mov eax, PAGE_TABLES_ADDR + 0x2000 ; PDPT_high
    or eax, 0x03
    mov [edi + 511 * 8], eax

    ; PDPT_low at PAGE_TABLES_ADDR + 0x1000
    ; Entry 0 -> PD_low (covers 0-1 GiB)
    mov edi, PAGE_TABLES_ADDR + 0x1000
    mov eax, PAGE_TABLES_ADDR + 0x3000 ; PD_low
    or eax, 0x03
    mov [edi], eax

    ; For entries 1..3 -> direct 1GiB pages (if supported) or more PDs
    ; Use a full PD for the first 1 GiB and PDPT huge for rest

    ; PDPT_high at PAGE_TABLES_ADDR + 0x2000
    ; Entry 510 -> PD_low (maps 0xFFFFFFFF80000000 -> physical 0x00000000)
    mov edi, PAGE_TABLES_ADDR + 0x2000
    mov eax, PAGE_TABLES_ADDR + 0x3000
    or eax, 0x03
    mov [edi + 510 * 8], eax

    ; PD_low at PAGE_TABLES_ADDR + 0x3000
    ; 512 entries of 2 MiB huge pages => maps 1 GiB
    mov edi, PAGE_TABLES_ADDR + 0x3000
    mov eax, 0x00000083 ; Present + Writable + PS (2MiB page)
    mov ecx, 512
.fill_pd:
    mov [edi], eax
    add eax, 0x200000 ; Next 2 MiB page
    add edi, 8
    loop .fill_pd

    ; Enable long mode
    ; Disable old paging first (should be off already)
    mov eax, cr0
    and eax, ~(1 << 31)
    mov cr0, eax

    ; Set CR3 to PML4
    mov eax, PAGE_TABLES_ADDR
    mov cr3, eax

    ; Enable PAE (CR4.PAE = bit 5)
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ; Set IA32_EFER.LME (Long Mode Enable) - MSR 0xC0000080, bit 8
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)
    wrmsr

    ; Enable paging (CR0.PG = bit 31) -> activates Long Mode
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64_ptr]

    ; Far jump to 64-bit code segment
    jmp 0x08:lm_entry

; 64-bit GDT
align 16
gdt64_start:
    ; Null
    dq 0
    ; 64-bit Code segment: L=1, D=0
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xAF, 0x00
    ; 64-bit Data segment
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00
    ; User Code segment (Ring 3): DPL=3
    dw 0xFFFF, 0x0000
    db 0x00, 0xFA, 0xAF, 0x00
    ; User Data segment (Ring 3): DPL=3
    dw 0xFFFF, 0x0000
    db 0x00, 0xF2, 0xCF, 0x00
    ; TSS descriptor (placeholder - filled by kernel)
    dq 0
    dq 0
gdt64_end:

gdt64_ptr:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start
    dd 0 ; High 32 bits of base (for 64-bit lgdt)

; long mode entry
[BITS 64]
lm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up 64-bit stack
    mov rsp, STACK64_TOP

    ; Pass BootInfo pointer as first argument (System V AMD64 ABI: RDI)
    mov rdi, BOOTINFO_ADDR

    ; Jump to kernel entry point at 1 MiB (physical, identity-mapped)
    ; _start in the kernel will remap to higher-half
    mov rax, KERNEL_PHYS
    jmp rax

    ; Should never reach here
    cli
    hlt

; Pad stage2 to fill allocated sectors
times (64 * 512) - ($ - $$) db 0