#include "kernel.h"

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

// Calculate the length of a string
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

// Copy memory from source to destination
void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

// Set memory to a specific value
void* memset(void* dest, int c, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    for (size_t i = 0; i < n; i++) {
        d[i] = (uint8_t)c;
    }
    return dest;
}

// Compare memory regions
int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    return 0;
}

// Copy a string
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

// Compare two strings
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Halt the CPU
void halt(void) {
    asm volatile ("cli\n\thlt");
}

// Panic function for unrecoverable errors
void panic(const char* message) {
    // We'll implement proper panic handling later
    asm volatile ("cli");
    while (1) {
        asm volatile ("hlt");
    }
}
