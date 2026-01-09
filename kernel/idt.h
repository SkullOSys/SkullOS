#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

// Segment selectors
#define KERNEL_CS 0x08

// How every interrupt gate (handler) is defined
typedef struct {
    uint16_t low_offset;  // Lower 16 bits of handler function address
    uint16_t sel;         // Kernel segment selector
    uint8_t always0;      // This must always be zero
    uint8_t flags;        // Flags (see below)
    uint16_t high_offset; // Higher 16 bits of handler function address
} __attribute__((packed)) idt_gate_t;

// A pointer to the array of interrupt handlers.
// Assembly instruction 'lidt' will read it
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256

// IDT flags
#define IDT_FLAG_PRESENT (1 << 7)
#define IDT_FLAG_RING0 (0 << 5)
#define IDT_FLAG_RING3 (3 << 5)
#define IDT_FLAG_32BIT_INTERRUPT 0xE
#define IDT_FLAG_32BIT_TRAP 0xF

// Function declarations
void idt_init(void);
void idt_set_gate(int n, uint32_t handler, uint16_t sel, uint8_t flags);
void idt_load(void);

// External assembly function to load the IDT
void idt_load_asm(uint32_t);

#endif // KERNEL_IDT_H
