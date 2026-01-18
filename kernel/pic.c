#include "pic.h"
#include "kernel.h"

// Initialize the Programmable Interrupt Controller (PIC)
void pic_init(void) {
    // Save masks
    uint8_t pic1_mask = inb(PIC1_DATA);
    uint8_t pic2_mask = inb(PIC2_DATA);
    
    // Start initialization sequence (cascade mode)
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    
    // Set vector offsets
    outb(PIC1_DATA, 0x20);  // Master PIC vectors start at 0x20 (32)
    outb(PIC2_DATA, 0x28);  // Slave PIC vectors start at 0x28 (40)
    
    // Tell Master PIC about the slave at IRQ2 (0000 0100)
    outb(PIC1_DATA, 0x04);
    // Tell Slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 0x02);
    
    // Set 8086/88 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // Restore saved masks
    outb(PIC1_DATA, pic1_mask);
    outb(PIC2_DATA, pic2_mask);
    
    // Enable IRQ0 (timer) and IRQ1 (keyboard)
    outb(PIC1_DATA, 0xFC);  // 1111 1100 - Enable IRQ0 (timer) and IRQ1 (keyboard)
    outb(PIC2_DATA, 0xFF);  // 1111 1111 - Disable all slave PIC interrupts
}

// Send End of Interrupt to PIC
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}
