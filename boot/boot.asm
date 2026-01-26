[org 0x7c00]
bits 16

jmp start

; Function to print a string
print_string:
    pusha
    mov ah, 0x0e    ; BIOS teletype output
.loop:
    lodsb           ; Load byte at DS:SI into AL
    or al, al       ; Check for null terminator
    jz .done
    int 0x10        ; BIOS video service
    jmp .loop
.done:
    popa
    ret

; GDT

;------------------------------------------------------------------------------
; detect_memory
; Detects system memory using BIOS interrupt 0x15, EAX=0xE820.
; Stores the memory map at 0x8500.
;------------------------------------------------------------------------------
detect_memory:
    pusha
    mov dword [0x8500], 0 ; Number of entries
    mov ebx, 0            ; EBX must be 0 for the first call
    mov di, 0x8504        ; ES:DI points to the buffer for the entries
.next_entry:
    mov eax, 0xe820       ; EAX = 0xE820
    mov edx, 0x534d4150   ; EDX = 'SMAP'
    mov ecx, 24           ; ECX = 24 bytes (size of the structure)
    int 0x15
    jc .error             ; If carry is set, there was an error

    cmp eax, 0x534d4150   ; Check for 'SMAP' signature
    jne .error

    add di, 24      ; The size of the returned structure
    cmp ebx, 0      ; if ebx is 0, we are done
    je .done

    inc dword [0x8500]
    jmp .next_entry

.error:
    mov dword [0x8500], 0
.done:
    popa
    ret

gdt_start:
    dq 0x0000000000000000  ; Null descriptor

gdt_code:
    ; Base=0x00000000, Limit=0xFFFFF, Present=1, DPL=00, Code=1, Conforming=0, Readable=1, Accessed=0, Granularity=1, 32-bit=1
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 0x9A         ; Access byte
    db 0xCF         ; Flags + Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_data:
    ; Base=0x00000000, Limit=0xFFFFF, Present=1, DPL=00, Code=0, Expand down=0, Writable=1, Accessed=0, Granularity=1, 32-bit=1
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 0x92         ; Access byte
    db 0xCF         ; Flags + Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT limit
    dd gdt_start                ; GDT base

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

start:
    ; Set up segment registers and stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; Stack grows down from 0x7C00
    sti

    ; Detect memory
    call detect_memory

    ; Save boot drive
    mov [boot_drive], dl

    ; Print boot message
    mov si, msg_booting
    call print_string

    ; Load kernel from disk
    mov ah, 0x02        ; BIOS read sectors function
    mov al, 64          ; Number of sectors to read (kernel is 64 sectors)
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2 (1-based)
    mov dh, 0           ; Head 0
    mov dl, [boot_drive]; Boot drive
    mov bx, 0x0000      ; ES:BX buffer address (ES=0x0000, BX=0x8000)
    mov es, bx
    mov bx, 0x8000
    int 0x13
    jnc load_initrd     ; Continue if no error

    mov si, msg_disk_error
    call print_string
    jmp $

load_initrd:
    ; Load initrd from disk
    mov ah, 0x02        ; BIOS read sectors function
    mov al, 1           ; Number of sectors to read
    mov ch, 1           ; Cylinder 1
    mov cl, 12          ; Sector 12
    mov dh, 1           ; Head 1
    mov dl, [boot_drive]; Boot drive
    mov bx, 0x2000      ; ES:BX buffer address (ES=0x2000, BX=0x0000)
    mov es, bx
    mov bx, 0x0000
    int 0x13
    jnc continue_boot   ; Continue if no error

    ; Print error message and halt if read failed
    mov si, msg_initrd_error
    call print_string
    jmp $

continue_boot:
    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to flush pipeline and load CS with 32-bit segment
    jmp CODE_SEG:p_mode_start

boot_drive: db 0

msg_booting: db "Booting from disk...", 0x0D, 0x0A, 0
msg_disk_error: db "Disk read error!", 0x0D, 0x0A, 0
msg_initrd_error: db "Initrd read error!", 0x0D, 0x0A, 0

bits 32
p_mode_start:
    ; Set up segment registers with data selector
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Set up stack
    mov esp, 0x90000
    
    ; Jump to kernel entry point
    jmp CODE_SEG:0x8000

times 510 - ($ - $$) db 0
dw 0xaa55
