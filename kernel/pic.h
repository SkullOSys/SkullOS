#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H

#include <stdint.h>

// PIC ports
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

// PIC commands
#define PIC_EOI    0x20  // End of Interrupt

// Initialize the PIC
void pic_init(void);

// Send End of Interrupt to PIC
void pic_send_eoi(uint8_t irq);

#endif // KERNEL_PIC_H
