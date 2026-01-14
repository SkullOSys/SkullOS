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

// Simple itoa implementation (helper function for integers to string)
char* itoa(int value, char* str, int base) {
    (void)base; // Base parameter is not used in this implementation
    
    char* ptr = str, *ptr1 = str, tmp_char;

    // Handle 0 explicitly, otherwise empty string is printed
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    // Process individual digits
    while (value != 0) {
        *ptr++ = '0' + (value % 10);
        value /= 10;
    }
    *ptr = '\0';

    // Reverse the string
    ptr--;
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return str;
}