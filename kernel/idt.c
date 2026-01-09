#include "idt.h"
#include "kernel.h"
#include "pic.h"

// Import the keyboard handler from the keyboard driver
extern void keyboard_handler_asm(void);

// Import the keyboard handler function
extern void keyboard_handler(void);

// IDT and IDT register
static idt_gate_t idt[IDT_ENTRIES];
static idt_register_t idt_reg;

// Set an IDT gate
void idt_set_gate(int n, uint32_t handler, uint16_t sel, uint8_t flags) {
    idt[n].low_offset = (handler & 0xFFFF);
    idt[n].sel = sel;
    idt[n].always0 = 0;
    idt[n].flags = flags | IDT_FLAG_PRESENT;
    idt[n].high_offset = (handler >> 16) & 0xFFFF;
}

// Initialize the IDT
void idt_init(void) {
    // Set up the IDT register
    idt_reg.limit = sizeof(idt_gate_t) * IDT_ENTRIES - 1;
    idt_reg.base = (uint32_t)&idt;
    
    // Clear out the entire IDT, initializing it to zeros
    memset(&idt, 0, sizeof(idt_gate_t) * IDT_ENTRIES);
    
    // Set up the keyboard interrupt (IRQ1 -> Interrupt 0x21)
    idt_set_gate(0x21, (uint32_t)keyboard_handler_asm, KERNEL_CS, IDT_FLAG_32BIT_INTERRUPT);
    
    // Load the IDT
    idt_load();
    
    // Initialize the PIC
    pic_init();
    
    // Enable interrupts
    asm volatile ("sti");
}

// Load the IDT
void idt_load(void) {
    idt_load_asm((uint32_t)&idt_reg);
}
