#include "timer.h"
#include "idt.h"
#include "../gui/gui.h"
#include "pic.h"
#include "kernel.h"

// Import the timer handler from interrupts.asm
extern void timer_handler_asm(void);

// Uptime tracking (in timer ticks, ~18.2 ticks per second)
static uint32_t uptime_ticks = 0;

// Timer interrupt handler
void timer_handler(void) {
    // Increment uptime
    uptime_ticks++;
    
    // Send End of Interrupt to the PIC
    pic_send_eoi(0);
    
    // Update GUI every ~18 ticks (approximately once per second)
    if (uptime_ticks % 18 == 0) {
        gui_draw_time();
        gui_draw_memory();
        gui_draw_uptime();
    }
}

// Get uptime in seconds
uint32_t timer_get_uptime_seconds(void) {
    return uptime_ticks / 18;  // Approximately 18.2 ticks per second
}

// Initialize the timer
void timer_init(void) {
    // Set up the timer interrupt (IRQ0 -> Interrupt 0x20)
    idt_set_gate(0x20, (uint32_t)timer_handler_asm, KERNEL_CS, IDT_FLAG_32BIT_INTERRUPT);
    
    // Configure PIT (Programmable Interval Timer) to generate interrupts at ~18.2 Hz
    // Channel 0, Mode 3 (square wave), Binary mode
    // Value 0xFFFF gives approximately 18.2 Hz (1193182 Hz / 65536)
    outb(0x43, 0x36);  // Command byte: Channel 0, Mode 3, Binary
    outb(0x40, 0x00);  // Low byte
    outb(0x40, 0x00);  // High byte
}
