[BITS 16]
[ORG 0x7C00]

start:
    ; Print "Hello, World!" using BIOS interrupt
    mov ah, 0x0E
    mov si, hello_msg
.print_char:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .print_char
.done:

get_a20_state:
	pushf
	push si
	push di
	push ds
	push es
	cli

	mov ax, 0x0000					;	0x0000:0x0500(0x00000500) -> ds:si
	mov ds, ax
	mov si, 0x0500

	not ax							;	0xffff:0x0510(0x00100500) -> es:di
	mov es, ax
	mov di, 0x0510

	mov al, [ds:si]					;	save old values
	mov byte [.BufferBelowMB], al
	mov al, [es:di]
	mov byte [.BufferOverMB], al

	mov ah, 1						;	check byte [0x00100500] == byte [0x0500]
	mov byte [ds:si], 0
	mov byte [es:di], 1
	mov al, [ds:si]
	cmp al, [es:di]
	jne .exit
	dec ah
.exit:
	mov al, [.BufferBelowMB]
	mov [ds:si], al
	mov al, [.BufferOverMB]
	mov [es:di], al
	shr ax, 8
	sti
	pop es
	pop ds
	pop di
	pop si
	popf
	ret
	
	.BufferBelowMB:	db 0
	.BufferOverMB	db 0

;	out:
;		ax - a20 support bits (bit #0 - supported on keyboard controller; bit #1 - supported with bit #1 of port 0x92)
;		cf - set on error
query_a20_support:
	push bx
	clc

	mov ax, 0x2403
	int 0x15
	jc .error

	test ah, ah
	jnz .error

	mov ax, bx
	pop bx
	ret
.error:
	stc
	pop bx
	ret

enable_a20_keyboard_controller:
	cli

	call .wait_io1
	mov al, 0xad
	out 0x64, al
	
	call .wait_io1
	mov al, 0xd0
	out 0x64, al
	
	call .wait_io2
	in al, 0x60
	push eax
	
	call .wait_io1
	mov al, 0xd1
	out 0x64, al
	
	call .wait_io1
	pop eax
	or al, 2
	out 0x60, al
	
	call .wait_io1
	mov al, 0xae
	out 0x64, al
	
	call .wait_io1
	sti
	ret
.wait_io1:
	in al, 0x64
	test al, 2
	jnz .wait_io1
	ret
.wait_io2:
	in al, 0x64
	test al, 1
	jz .wait_io2
	ret

;	out:
;		cf - set on error
enable_a20:
	clc									;	clear cf
	pusha
	mov bh, 0							;	clear bh

	call get_a20_state
	jc .fast_gate

	test ax, ax
	jnz .done

	call query_a20_support
	mov bl, al
	test bl, 1							;	enable A20 using keyboard controller
	jnz .keybord_controller

	test bl, 2							;	enable A20 using fast A20 gate
	jnz .fast_gate
.bios_int:
	mov ax, 0x2401
	int 0x15
	jc .fast_gate
	test ah, ah
	jnz .failed
	call get_a20_state
	test ax, ax
	jnz .done
.fast_gate:
	in al, 0x92
	test al, 2
	jnz .done

	or al, 2
	and al, 0xfe
	out 0x92, al

	call get_a20_state
	test ax, ax
	jnz .done

	test bh, bh							;	test if there was an attempt using the keyboard controller
	jnz .failed
.keybord_controller:
	call enable_a20_keyboard_controller
	call get_a20_state
	test ax, ax
	jnz .done

	mov bh, 1							;	flag enable attempt with keyboard controller

	test bl, 2
	jnz .fast_gate
	jmp .failed
.failed:
	stc
.done:
	popa
	ret

gdt_start:
    dq 0x0000000000000000        ; Null descriptor
    dq 0x00CF9A000000FFFF        ; Code segment (32-bit)
    dq 0x00CF92000000FFFF        ; Data segment (32-bit)
    dq 0x00CFFA000000FFFF        ; Code segment (64-bit)
    dq 0x00CFF2000000FFFF        ; Data segment (64-bit)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; Size of the GDT (limit)
    dd gdt_start                 ; Address of the GDT

; Set up GDT and enable protected mode
setup_gdt:
    lgdt [gdt_descriptor]        ; Load the GDT with the `lgdt` instruction
    ret

call query_a20_support
call enable_a20
call setup_gdt

; Jump to extended bootloader in the next 512+ bytes
jmp 0x7E00:bootloader_start

hello_msg db 'Hello World!', 0

times 510 - ($ - $$) db 0  ; Pad to 510 bytes
dw 0xAA55                   ; Boot signature
; From here is 0x7E00, we will jump further in memory (0xF000) when our actual stage 2 loader is in memory.
bootloader_start:
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	[BITS 32]

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, 0x8F00
	jmp load_disk_sector

	; Switch back to real mode for just a second, so we have the BIOS function 0x13
rm_switch:
	cli

	; clear the PE bit
	mov eax, cr0
	and eax, 0xFFFFFFFE
	mov cr0, eax

	; reload segment registers to real mode segments
	mov ax, 0x07C0
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00
	jmp 0x07C0:startRealMode ; far jump to real mode, this is only to flush the CPU pipeline
	ret

startRealMode:
	ret                      ; as shown here

[bits 16]
pm_switch_2:
	cli
	lgdt [gdt_descriptor]    ; reload GDT

	; set PE bit in CR0 to re-enable protected mode
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp 0x08:pmode_entry
	ret

disk_read:
	[bits 16]               ; to be safe during assembly
	; set disk read parameters
	mov ah, 0x02            ; read sector function
	mov al, 0x0F            ; sectors to read (16)
	mov ch, 0x00            ; cylinder number
	mov cl, 0x02            ; sector number
	mov dh, 0x00            ; head number
	mov dl, 0x80            ; drive number (0x80, first HDD)
	int 0x13                ; disk services interrupt

	jc .disk_err

	; success!
	ret
.disk_err:
	cli                     ; disable interrupts
	hlt                     ; halt CPU
	ret

[bits 32]
pmode_entry:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, 0xEFFF         ; empty location, right before our loader, so it grows downwards
load_disk_sector:
	; save the current CPU state
	pusha
	push ds
	push es

	; sector to load
	mov bx, 0xF000          ; destination address
	mov dl, 0x80            ; drive number (0x00 for floppy, 0x80 for first hdd)
	mov dh, 0               ; head number
	mov ch, 0               ; cylinder number
	mov cl, 2               ; sector number (start from sector 2)

	; load sector(s) from disk
	call rm_switch
	call disk_read
	call pm_switch_2

	; restore CPU state
	pop es
	pop ds
	popa
	
	jmp 0x08:0xF000        ; jump to the actual stage 2 loader