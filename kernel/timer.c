#include "timer.h"
#include "idt.h"
#include "../gui/gui.h"
#include "pic.h"
#include "kernel.h"

// Import the timer handler from interrupts.asm
extern void timer_handler_asm(void);

// Timer interrupt handler
void timer_handler(void) {
    // Send End of Interrupt to the PIC
    pic_send_eoi(0);
    
    // Call the GUI time drawing function
    gui_draw_time();
}

// Initialize the timer
void timer_init(void) {
    // Set up the timer interrupt (IRQ0 -> Interrupt 0x20)
    idt_set_gate(0x20, (uint32_t)timer_handler_asm, KERNEL_CS, IDT_FLAG_32BIT_INTERRUPT);
}
