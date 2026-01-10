#ifndef KERNEL_VGA_H
#define KERNEL_VGA_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// VGA text mode buffer address
#define VGA_MEMORY (uint16_t*)0xB8000
#define VGA_WIDTH 80

// Terminal functions
void terminal_initialize(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_puts(const char* str);
void terminal_set_color(uint8_t fg, uint8_t bg);
void vga_set_color(enum vga_color fg, enum vga_color bg);
#define VGA_HEIGHT 25

// Function prototypes
void vga_initialize(void);
void vga_clear_screen(void);
void vga_set_color(enum vga_color fg, enum vga_color bg);
void vga_put_char(char c);
void vga_put_string(const char* str);
void vga_set_cursor_pos(int x, int y);
void vga_get_cursor_pos(int* x, int* y);
void vga_scroll(void);

// Terminal functions (wrapper around VGA functions)
void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_puts(const char* data);
void terminal_put_hex(uint32_t num);
void terminal_put_dec(uint32_t num);

#endif // KERNEL_VGA_H
