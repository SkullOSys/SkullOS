; Interrupt service routine stubs
[bits 32]

; Import the C handler functions
extern keyboard_handler
extern timer_handler

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
