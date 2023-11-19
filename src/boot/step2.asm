section .data
    cpu_info_str db "CPU: ", 0
    ram_info_str db "RAM: ", 0
    unsupported_cpu_msg_str db "Error: Unsupported CPU. Press any key to turn off the computer.", 0
    unknown_error_msg db "An unknown error has occured.", 0
    ram_error_msg db "Error: ComputiOS was unable to map enough RAM to use ComputiOS. Press any key to turn off the computer.", 0

section .bss
    read_buffer resb 512   ; Buffer for reading sectors

section .text
    global step2_start

step2_start:
    ; Display CPU and RAM information
    call display_cpu_info
    call display_ram_info

    ; Check hardware requirements
    call check_hardware

    ; Continue to Step 3 if hardware requirements are met
    jmp loadStep3

display_cpu_info:
    ; Use CPUID to get CPU information
    mov eax, 0           ; CPUID function to get vendor ID
    cpuid

    ; Print CPU information
    mov esi, cpu_info_str
    call print_string

    ; Print the vendor ID
    mov esi, ebx
    call print_string

    ; Newline
    mov eax, 0x0A0D
    mov ebx, 0x0007
    int 0x10

    ret

display_ram_info:
    ; Print RAM information
    mov esi, ram_info_str
    call print_string

    ; Use INT 0x15, AX=0xE820 to get the memory map
    mov eax, 0xE820
    mov ebx, 24
    mov edx, 0x534D4150
    mov ecx, 0
    int 0x15

    jc  error_memory_ram_info    ; Jump to error handling if the carry flag is set

    ; Check the memory map entries for total usable RAM
    mov esi, 0x0
    mov edi, 0          ; Initialize total memory counter

.next_entry_ram_info:
    mov eax, [esi]
    cmp eax, 1           ; Check if it's a usable memory range
    jne .not_usable_ram_info

    ; Add the size of the usable range to the total memory counter
    add edi, [esi + 8]

.not_usable_ram_info:
    add esi, ebx         ; Move to the next memory map entry
    jmp .check_next_entry_ram_info

.check_next_entry_ram_info:
    cmp dword [esi], edx ; Check if the signature matches "SMAP"
    jne error_memory_ram_info

    jmp .next_entry_ram_info_2

.next_entry_ram_info_2:
    ; Print the total usable RAM
    mov eax, edi         ; Load the total usable memory size
    mov ebx, 1024        ; Convert from kilobytes to megabytes
    div ebx              ; EAX contains the result in megabytes
    mov esi, eax
    call print_number    ; Print the total usable memory in megabytes

    ; Newline
    mov eax, 0x0A0D
    mov ebx, 0x0007
    int 0x10

    ret

check_hardware:
    ; Check processor speed
    call check_processor_speed
    jc  error_unsupported_cpu   ; Jump to error handling if the carry flag is set

    ; Check available memory
    call check_available_memory
    jc  unk_error   ; Jump to error handling if the carry flag is set

    ret

check_processor_speed:
    ; Check processor speed using INT 0x15, EAX=0
    mov eax, 0          ; INT 0x15, EAX=0 (Get System Time)
    int 0x15            ; BIOS interrupt

    ; The result is in EAX, containing the processor speed in kHz
    ; You can compare it against your required speed
    ; Example: If you need a minimum of 1 GHz (1000 MHz), you can check if EAX >= 1000000
    cmp eax, 1000000    ; Adjust the value as needed
    jb error_speed      ; Jump to error handling if below the required speed

    ret

check_available_memory:
    ; Check available memory using INT 0x15, EAX=0xE820
    mov eax, 0xE820     ; INT 0x15, EAX=0xE820 (Get Memory Map)
    mov ebx, 24         ; Size of memory map entry
    mov edx, 0x534D4150 ; Signature "SMAP" in ASCII
    mov ecx, 0          ; Start from the beginning of the memory map
    int 0x15            ; BIOS interrupt

    jc  error_memory    ; Jump to error handling if the carry flag is set

    ; (Process the memory map as needed)

    ret

error_memory:
    mov esi, ram_error_msg
    call print_string

    ; Wait for keypress to turn off the computer
    mov ah, 0           ; AH=0, check for keypress
    int 0x16            ; BIOS keyboard services
    cmp ah, 0           ; Check if key was pressed
    jne turn_off        ; If so, turn off the computer.
    
    ret

error_speed:
    mov esi, unsupported_cpu_msg_str
    call print_string

    ; Wait for keypress to turn off the computer
    mov ah, 0           ; AH=0, check for keypress
    int 0x16            ; BIOS keyboard services
    cmp ah, 0           ; Check if key was pressed
    jne turn_off        ; If so, turn off the computer.
    
    ret

error_unsupported_cpu:
    mov esi, unsupported_cpu_msg_str
    call print_string

    ; Wait for keypress to turn off the computer
    mov ah, 0           ; AH=0, check for keypress
    int 0x16            ; BIOS keyboard services
    cmp ah, 0           ; Check if key was pressed
    jne turn_off        ; If so, turn off the computer.
    
    ret

error_memory_ram_info:
    mov esi, ram_error_msg
    call print_string

    ; Wait for keypress to turn off the computer
    mov ah, 0           ; AH=0, check for keypress
    int 0x16            ; BIOS keyboard services
    cmp ah, 0           ; Check if key was pressed
    jne turn_off       ; If so, turn off the computer.

    ret

unk_error:
    mov esi, unknown_error_msg
    call print_string

    cli                ; Disable interrupts
    hlt                ; Halt the processor

turn_off:
    ; Turn off the computer
    mov ax, 0x5307     ; AX=0x5307 (Shutdown)
    mov bx, 0x0001     ; BX=0x0001 (Power off)
    mov cx, 0x0003     ; CX=0x0003 (Reserved, must be set)
    int 0x15           ; BIOS power-off services

    ; If the above method fails, you can use INT 0x16, AH=0 to wait indefinitely
    ; and then use INT 0x19 to reboot the computer.

    cli                ; Disable interrupts
    hlt                ; Halt the processor

print_string:
    ; Print a null-terminated string at the address in ESI
    .print_loop:
        lodsb            ; Load the next byte from the string
        cmp al, 0        ; Check if it's the null terminator
        je .print_done   ; If yes, finish printing
        mov eax, 0x0E00  ; AH=0x0E (Teletype output), AL=character
        mov ebx, 0x0007  ; BL=display page
        int 0x10         ; BIOS video services
        jmp .print_loop  ; Repeat the loop

    .print_done:
        ret

print_number:
    ; Print a number in ESI
    mov ecx, 10         ; Set divisor to 10
    xor ebx, ebx        ; Clear EBX to store the reversed digits

    .reverse_loop:
        div ecx          ; Divide ESI by 10, result in EAX, remainder in EDX
        add dl, '0'      ; Convert remainder to ASCII
        push dx          ; Save ASCII character on the stack
        test eax, eax    ; Check if quotient is zero
        jnz .reverse_loop ; If not, continue the loop

    .pop_and_print_loop:
        pop dx           ; Pop the ASCII character from the stack
        mov eax, 0x0E00  ; AH=0x0E (Teletype output), AL=character
        mov ebx, 0x0007  ; BL=display page
        int 0x10         ; BIOS video services
        test dx, dx      ; Check if this is the last character
        jnz .pop_and_print_loop ; If not, continue the loop

    ret

loadStep3:
