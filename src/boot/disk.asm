;---------------------------------------------------
; Read sectors subroutine
; Read sectors from disk starting at a given LBA.
; Expects:
;   AX:DX = Starting LBA (sector address)
;   ES:DI = Target memory address
;   CX    = Number of sectors to read
; Returns:
;   None
;---------------------------------------------------
readSectors:
    push ax
    push bx
    push cx
    push dx
    push di
    push es

    mov bx, di                                  ; Load ES:BX as the destination address

.sectorLoop:
    push ax
    push cx
    push dx

    div word [sectorsPerTrack]                  ; LBA ÷ sectors/track -> CX (track), DX (sector)
    mov cx, dx                                  ; CX = absolute sector + 1
    inc cx

    xor dx, dx                                  ; LBA ÷ heads -> AL (track), DL (head)
    div word [heads]
    mov dh, dl                                  ; DH = absolute head
    mov ch, al                                  ; CH = absolute track (low 8 bits)
    mov cl, 6                                   ; Shift for track's high 2 bits
    shl ah, cl
    or ch, ah                                   ; Combine track's high bits

    mov dl, byte [cs:drive]                     ; Use boot drive
    mov ah, 0x02                                ; INT 13h function to read sectors
    mov al, 1                                   ; Read one sector at a time

    mov di, 5                                   ; Retry up to 5 times
.readAttempt:
    int 0x13                                    ; BIOS interrupt for disk I/O
    jnc .readOk                                 ; If successful, continue
    dec di
    jnz .readAttempt                            ; Retry reading
    jmp error                                   ; If unsuccessful after retries, error

.readOk:
    pop dx
    pop cx
    pop ax

    clc
    add ax, 1                                   ; Increment LBA
    adc dx, 0

    clc
    add bx, word [bytesPerSector]               ; Advance buffer address
    jnc .nextSector

.fixBuffer:
    mov ax, es                                  ; Correct buffer crossing 64k boundary
    add ax, 0x10
    mov es, ax

.nextSector:
    loop .sectorLoop                            ; Repeat for remaining sectors

    pop es
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret


;---------------------------------------------------
; Read clusters subroutine for FAT32
; Read all clusters of a file from the FAT chain.
; Expects:
;   AX    = Starting cluster number
;   DS:SI = Address of the FAT
;   ES:DI = Target memory address
; Returns:
;   None
;---------------------------------------------------
readClusters:
.clusterLoop:
    push ax                                    ; Save the current cluster

    ; Calculate first sector of cluster
    mov dx, ax
    sub dx, 2                                 ; Clusters start at 2
    xor bx, bx
    mov bl, byte [sectorsPerCluster]          ; Multiply by sectors per cluster
    mul bx
    add ax, word [startOfData]                ; Add starting data sector
    call readSectors                          ; Read the sectors for the cluster

    pop ax                                    ; Restore current cluster

    ; Get next cluster from FAT
    mov edx, eax                              ; EDX = Current cluster
    mov ecx, 4                                ; Multiply by 4 (FAT32 entries are 4 bytes)
    mul ecx
    add si, ax                                ; Point to the FAT entry
    mov eax, [ds:si]                          ; Load the FAT32 entry
    and eax, 0x0FFFFFFF                       ; Mask the high 4 bits (FAT32-specific)

    cmp eax, 0x0FFFFFF8                       ; Check if it's the last cluster
    jae .done                                 ; If end of file, stop

    ; Advance memory buffer pointer
    xor dx, dx
    xor ah, ah
    mov al, byte [sectorsPerCluster]          ; Calculate bytes per cluster
    mul word [bytesPerSector]
    add di, ax                                ; Advance ES:DI
    jc .fixBuffer                             ; Handle 64k boundary crossing

    jmp .clusterLoop                          ; Process the next cluster

.fixBuffer:
    mov ax, es                                ; Correct segment for 64k boundary
    add ax, 0x10
    mov es, ax
    jmp .clusterLoop

.done:
    ret