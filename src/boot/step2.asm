section .data
    cpu_info db "CPOS_CPU: ", 0
    ram_info db "CPOS_RAM: ", 0

section .text
    global checkHwReq

checkHwReq:
    ; Check RAM size
    call checkRAM
    
    ; Check CPU speed
    call checkCPU

    ; Display CPU and RAM info
    call displayCPUinfo
    call displayRAMinfo

    ; Move to step 3
    jmp loadStep3

displayCPUinfo:
    ; Use CPUID to get CPU info
    mov eax, 0                     ; CPUID function to get vendor ID
    cpuid

    ; Print CPU information
    mov esi, cpu_info
    call printString

    ; Print Vendor ID
    mov esi, ebx
    call printString

    ; New line
    mov eax, 0x0A0D
    mov ebx, 0x0007
    int 0x10

    ret

displayRAMinfo:
    ; Print RAM info
    mov esi, ram_info
    call printString

    ; Use int 0x15, ax=0xE820 to get memory map
    mov eax, 0xE820
    mov ebx, 24
    mov edx, 0x534D4150
    mov ecx, 0
    int 0x15

    jc .error_memory               ; Jump to error handling if carry flag is set
    
    ; Check the memory map entries for total usable RAM
    mov esi, 0x0
    mov edi, 0                     ; Init total memory counter
.next_entry:
    mov eax, [esi]
    cmp eax, 1                     ; Check if it's a usable memory range
    jne .not_usable

    ; Add the size of the usable range to the total memory counter
    add edi, [esi + 8]

.check_not_usable:
    add esi, ebx
    jmp .next_entry_check

.next_entry_check:
    cmp dword [esi], edx           ; Check if signature matches "SMAP"
    jne .memErr

    jmp .printTotalRAM

.printTotalRAM:
    ; Print total usable RAM
    mov eax, edi                   ; Load the total usable memory size
    mov ebx, 1024                  ; Convert from KiB to MiB
    div ebx                        ; EAX contains the results in MiB
    mov esi, eax
    call printNumber               ; Print total usable RAM in MiB

    ; New line
    mov eax, 0x0A0D
    mov ebx, 0x0007
    int 0x10

    ret

checkRAM:
    mov eax, 0xE820                ; BIOS memory map function
    mov ebx, 24                    ; Size of memory map entry
    mov edx, 0x534D4150            ; Signature "SMAP" in ASCII
    mov ecx, 0                     ; Start with entry 0

    int 0x15                       ; BIOS function 15h
    
    jc .error                      ; Show error if carry flag is set

    ; Check the memory map entries for available RAM
    mov esi, 0x0                   ; Start with entry 0
.next_RAM_map_entry:
    mov eax, [esi]                 ; Load the memory map entry type
    cmp eax, 1                     ; Check if it's usable memory range
    jne .not_usable_ram

    ; Check if the range size is at least 512MB
    mov eax, [esi + 8]             ; Load the size of the memory range in kilobytes
    cmp eax, 0x80000               ; 1MB = 0x1000, 512MB = 0x80000
    jge .usable_range

.not_usable:
    add esi, ebx                   ; Move to next memory map entry
    jmp .check_next_entry

.usable_range:
    ; RAM req. is met
    ret

.check_next_entry:
    cmp dword [esi], edx           ; Check if the signature matches "SMAP"
    jne .error_memory

    jmp .next_RAM_map_entry

.error_memory:
    stc                            ; Set carry flag to indicate error
    ret

checkCPU:
    ; Use CPUID to check processor speed (in MHz.)
    mov eax, 0x80000002            ; CPUID function to get processor brand string
    cpuid

    ; Check if CPU speed is at least 1GHz.
    cmp eax, 0x1000                ; 1MHz * 1000 = 1GHz.
    jl .error_speed                ; Jump to error if the speed is less than 1GHz.

    ret

.error_speed:
    stc
    ret

printString:
    ; Print a null-terminated string at the address in ESI
    .printLoop:
        lodsb                      ; Load the next byte from the string
        cmp al, 0                  ; check if it's the null terminator
        je .print_done             ; If so, finish printing
        mov eax, 0x0E00            ; AH=0x0E (Teletype output), AL=Character
        mov ebx, 0x0007            ; BL=display page
        int 0x10                   ; BIOS video services
        jmp .print_loop            ; Repeat loop

    .print_done:
        ret

printNumber:
    ; print a number in ESI
    mov ecx, 10                    ; Set divisor to 10
    xor ebx, ebx                   ; Clear EBX to store the reversed digits

    .reverse_loop:
        div ecx                    ; Divide ESI by 10, result in EAX, remainder in EDX
        add dl, '0'                ; Convert remainder to ASCII
        push dx                    ; Save ASCII characters to the stack
        test eax, eax              ; Check if quotient is 0
        jnz .reverse_loop          ; If not, continue the loop

    .pop_and_print_loop:
        pop dx                     ; Pop the ASCII characters from the stack
        mov eax, 0x0E00            ; AH=0x0E (Teletype output), AL=character
        mov ebx, 0x0007            ; BL=display page
        int 0x10                   ; BIOS video services
        test dx, dx                ; Check if this is the last character
        jnz .pop_and_print_loop    ; If not, continue the loop
    
    ret

loadStep3:
    