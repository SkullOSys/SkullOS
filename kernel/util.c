#include "util.h"

// Output a byte to the specified port
void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Read a byte from the specified port
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Halt the CPU
void halt(void) {
    asm volatile ("cli\n\thlt");
}

// Panic function for unrecoverable errors
void panic(const char* message) {
    (void)message;
    // We'll implement proper panic handling later
    asm volatile ("cli");
    while (1) {
        asm volatile ("hlt");
    }
}
