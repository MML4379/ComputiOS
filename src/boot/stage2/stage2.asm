; -----------------------------------------------------
; | ComputiOS Stage 2 Loader                          |
; | This loads the kernel from a FAT32 filesystem,    |
; | sets up 64-bit long mode with paging, and passes  |
; | control to the kernel.                            |
; -----------------------------------------------------

[BITS 32]
[ORG 0x1000]

%define PAGE_TABLE_BASE   0x2000          ; Page table base address
%define KERNEL_LOAD_ADDR  0x400000        ; Kernel entry point address
%define ROOT_CLUSTER      2               ; Root directory cluster number
%define BYTES_PER_SECTOR  512             ; Size of each sector in bytes
%define FILENAME          "CPOSKRNL.MAPP" ; Kernel file name

%inc "src/boot/disk.asm"

section .text
global _start
_start:
    cli                                 ; Disable interrupts
    cld                                 ; Clear direction flag

    ; Step 1: Load the kernel
    call load_kernel

    ; Step 2: Setup GDT
    call setup_gdt

    ; Step 3: Enable PAE and Long Mode
    call enable_long_mode

    ; Step 4: Setup Paging
    call setup_paging

    ; Step 5: Jump to kernel in 64-bit mode
    call enter_long_mode

;---------------------------------------------------
; Load the kernel from FAT32
;---------------------------------------------------

load_kernel:
    ; Locate the kernel in the root directory
    mov ax, ROOT_CLUSTER                ; Start with the root directory cluster
    mov ds, [fat_table_address]         ; Point to the FAT table
    mov es, KERNEL_LOAD_ADDR >> 4       ; Set kernel load segment

    call find_file                      ; Locate the file
    jc error                            ; If not found, halt

    mov ax, [kernel_cluster]            ; Load kernel's starting cluster
    call readClusters                   ; Load all clusters into memory
    ret

find_file:
    ; Locate the file in the directory and retrieve starting cluster
    mov bx, ax                          ; Starting cluster
    xor cx, cx                          ; Clear cluster offset
    mov si, directory_buffer            ; Directory entries buffer

.dir_loop:
    push bx
    call readClusters                   ; Read directory cluster into memory

    mov di, si                          ; Start parsing entries
    mov cx, BYTES_PER_SECTOR
    shr cx, 5                           ; 32 bytes per directory entry

.search_entry:
    mov di, si                          ; Reset DI to start of current entry
    lodsb                               ; Load first byte of the entry
    cmp al, 0x00                        ; End of directory
    je .file_not_found
    cmp al, 0xE5                        ; Skip deleted entries
    je .next_entry
    cmp byte [si + 11], 0x0F            ; Skip long filename entries
    je .next_entry

    ; Compare filename (11 bytes)
    mov di, si
    mov si, filename_table
    mov cx, 11
    repe cmpsb
    jz .file_found                      ; Match found

.next_entry:
    add si, 32                          ; Move to next entry
    loop .search_entry

    ; Move to the next cluster
    pop bx
    mov ax, bx
    call next_cluster                   ; Get next cluster in chain
    jc .file_not_found
    jmp .dir_loop

.file_found:
    ; Retrieve starting cluster
    mov ax, [si + 26]                   ; Low 16 bits
    mov dx, [si + 20]                   ; High 16 bits
    shl edx, 16
    or eax, edx
    mov [kernel_cluster], eax
    ret

.file_not_found:
    stc
    ret

;---------------------------------------------------
; GDT Setup for Long Mode
;---------------------------------------------------

setup_gdt:
    lgdt [gdt_descriptor]
    ret

;---------------------------------------------------
; Enable Long Mode
;---------------------------------------------------

enable_long_mode:
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5                      ; Set PAE bit
    mov cr4, eax

    ; Enable Long Mode in IA32_EFER MSR
    mov ecx, 0xC0000080                 ; IA32_EFER MSR
    rdmsr
    or eax, 1 << 8                      ; Set LME (Long Mode Enable)
    wrmsr

    ret

;---------------------------------------------------
; Setup Paging
;---------------------------------------------------

setup_paging:
    ; Set up PML4 table, PDPT, and Page Directory (identity map first 2MB)
    mov eax, PAGE_TABLE_BASE
    mov [eax], eax
    or dword [eax], 0x83                ; Present, writable, PAE

    ; Set up PDPT
    add eax, 0x1000
    mov [PAGE_TABLE_BASE + 8], eax
    or dword [PAGE_TABLE_BASE + 8], 0x83

    ; Set up Page Directory
    add eax, 0x1000
    mov [PAGE_TABLE_BASE + 0x1008], eax
    or dword [PAGE_TABLE_BASE + 0x1008], 0x83

    ; Set up first 2MB page
    add eax, 0x1000
    mov [PAGE_TABLE_BASE + 0x2008], eax
    or dword [PAGE_TABLE_BASE + 0x2008], 0x83

    ; Enable Paging
    mov eax, PAGE_TABLE_BASE
    mov cr3, eax                        ; Load PML4 base address

    mov eax, cr0
    or eax, 0x80000001                  ; Enable Paging and Protected Mode
    mov cr0, eax

    ret

;---------------------------------------------------
; Enter 64-bit Long Mode
;---------------------------------------------------

enter_long_mode:
    jmp 0x8:long_mode_entry             ; Far jump to 64-bit mode

[BITS 64]
long_mode_entry:
    mov ax, 0x10                        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump to kernel
    mov rdi, KERNEL_LOAD_ADDR           ; Pass kernel entry address in RDI
    call [rdi]

    hlt                                 ; Halt if kernel returns

;---------------------------------------------------
; GDT
;---------------------------------------------------

align 8
gdt:
    dq 0x0000000000000000               ; Null descriptor
    dq 0x00AF9A000000FFFF               ; Code segment
    dq 0x00AF92000000FFFF               ; Data segment

gdt_descriptor:
    dw gdt_end - gdt - 1
    dd gdt
gdt_end:

;---------------------------------------------------
; Variables and Buffers
;---------------------------------------------------

section .data
filename_table db 'CPOSKRNL', 'MAP', 0  ; FAT32 filename
kernel_cluster dd 0                    ; Kernel's starting cluster

section .bss
directory_buffer resb 4096             ; Directory entries buffer
fat_table_address dd 0                 ; FAT table address placeholder
