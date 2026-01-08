#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Kernel entry point (defined in kernel.c)
void kernel_entry(void);

// Main kernel function
void kernel_main(void);

// Basic I/O functions
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);

// String functions
size_t strlen(const char* str);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
char* strcpy(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);

// Panic function for unrecoverable errors
void panic(const char* message);

// Halt the CPU
void halt(void);

#endif // KERNEL_H
