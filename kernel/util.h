#ifndef KERNEL_UTIL_H
#define KERNEL_UTIL_H

#include <stdint.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
char* itoa(int value, char* str, int base);

#endif // KERNEL_UTIL_H
