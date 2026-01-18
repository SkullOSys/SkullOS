; Interrupt service routine stubs
[bits 32]

; Import the C handler functions
extern keyboard_handler
extern timer_handler
extern syscall_handler

global idt_load_asm
idt_load_asm:
    mov eax, [esp + 4]  ; Get the pointer to the IDT, passed as a parameter
    lidt [eax]          ; Load the IDT pointer
    ret

; Keyboard interrupt handler stub
global keyboard_handler_asm
keyboard_handler_asm:
    ; Save all general-purpose registers
    pushad
    
    ; Call the C handler
    call keyboard_handler
    
    ; Restore all general-purpose registers
    popad
    
    ; Return from interrupt
    iret

; Timer interrupt handler stub
global timer_handler_asm
timer_handler_asm:
    ; Save all general-purpose registers
    pushad
    
    ; Call the C handler
    call timer_handler
    
    ; Restore all general-purpose registers
    popad
    
    ; Return from interrupt
    iret

; System call interrupt handler (INT 0x80)
global syscall_handler_asm
syscall_handler_asm:
    ; Save all registers (pushad saves: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
    pushad
    
    ; Arguments are already in registers:
    ; eax = syscall number
    ; ebx = arg1
    ; ecx = arg2
    ; edx = arg3
    
    ; Call the C handler (it will read from saved registers on stack)
    call syscall_handler
    
    ; Return value is in eax, which was modified by the handler
    ; The handler stored the result in the saved eax position on stack
    ; Restore all registers (including modified eax)
    popad
    
    ; Return from interrupt
    iret
