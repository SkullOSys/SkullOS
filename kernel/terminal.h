#ifndef KERNEL_TERMINAL_H
#define KERNEL_TERMINAL_H

#include <stddef.h>
#include <stdint.h>

void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_puts(const char* data);
void terminal_put_hex(uint32_t num);
void terminal_put_dec(uint32_t num);

#endif // KERNEL_TERMINAL_H
