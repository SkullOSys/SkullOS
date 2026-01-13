#ifndef KERNEL_VGA_MANAGER_H
#define KERNEL_VGA_MANAGER_H

#include "vga.h"

typedef struct {
    int cursor_x;
    int cursor_y;
    uint8_t color;
    int width;
    int height;
    int start_x;
    int start_y;
} vga_context_t;

void vga_manager_init(void);
void vga_manager_set_context(bool is_gui);

void vga_manager_putchar(char c);
void vga_manager_puts(const char* str);
void vga_manager_clear(void);
void vga_manager_set_color(enum vga_color fg, enum vga_color bg);

void vga_manager_fullscreen_clear(void);
void vga_manager_fullscreen_set_color(enum vga_color fg, enum vga_color bg);
void vga_manager_fullscreen_set_cursor_pos(int x, int y);
void vga_manager_fullscreen_puts(const char* str);
void vga_manager_fullscreen_putchar(char c);

#endif // KERNEL_VGA_MANAGER_H
