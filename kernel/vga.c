#include "vga.h"
#include "kernel.h"
#include "vga_manager.h"
#include "terminal.h"
#include <string.h>

void terminal_putchar(char c) {
    vga_manager_putchar(c);
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_puts(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_put_hex(uint32_t num) {
    const char* hex_digits = "0123456789ABCDEF";
    char buf[11]; // 0x + 8 digits + null terminator
    
    buf[0] = '0';
    buf[1] = 'x';
    
    for (int i = 0; i < 8; i++) {
        buf[9 - i] = hex_digits[num & 0xF];
        num >>= 4;
    }
    
    buf[10] = '\0';
    terminal_puts(buf);
}

void terminal_put_dec(uint32_t num) {
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buf[11]; // Maximum 10 digits for 32-bit number + null terminator
    int i = 0;
    
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Print in reverse order
    while (--i >= 0) {
        terminal_putchar(buf[i]);
    }
}
