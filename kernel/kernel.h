#ifndef KERNEL_H
#define KERNEL_H

// Include our minimal libc headers first
#include "libc/include/stdint.h"
#include "libc/include/stddef.h"
#include "libc/include/stdbool.h"
#include "libc/include/string.h"

// Kernel entry point (defined in kernel.c)
void kernel_entry(void);

// Main kernel function
void kernel_main(void);

// Basic I/O functions
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);

// String functions (now in libc/include/string.h)

// Panic function for unrecoverable errors
void panic(const char* message);

// Halt the CPU
void halt(void);

// Memory allocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);

// Memory management
void memory_init(void);

#endif // KERNEL_H