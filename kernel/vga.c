#include "vga.h"
#include "kernel.h"

// Current cursor position
static int cursor_x = 0;
static int cursor_y = 0;

// Current color attributes
static uint8_t vga_color = 0x0F; // White on black by default

// VGA text mode buffer
static uint16_t* const vga_buffer = VGA_MEMORY;

// Private function declarations
static void update_cursor(void);
static uint16_t vga_entry(unsigned char uc, uint8_t color);

// Initialize VGA text mode
void vga_initialize(void) {
    vga_clear_screen();
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_set_cursor_pos(0, 0);
}

// Clear the screen
void vga_clear_screen(void) {
    const uint16_t blank = vga_entry(' ', vga_color);
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

// Set the current color
void vga_set_color(enum vga_color fg, enum vga_color bg) {
    vga_color = fg | (bg << 4);
}

// Put a character at the current cursor position
void vga_put_char(char c) {
    // Handle newline
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', vga_color);
        }
    } else if (c >= ' ') {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, vga_color);
        cursor_x++;
    }

    // Handle line wrap and scroll
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Scroll if needed
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
        cursor_y = VGA_HEIGHT - 1;
    }
    
    update_cursor();
}

// Put a string at the current cursor position
void vga_put_string(const char* str) {
    while (*str) {
        vga_put_char(*str++);
    }
}

// Set cursor position
void vga_set_cursor_pos(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
        update_cursor();
    }
}

// Get cursor position
void vga_get_cursor_pos(int* x, int* y) {
    if (x) *x = cursor_x;
    if (y) *y = cursor_y;
}

// Scroll the screen up by one line
void vga_scroll(void) {
    // Move all lines up by one
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[(y - 1) * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    
    // Clear the last line
    const uint16_t blank = vga_entry(' ', vga_color);
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
}

// Update the hardware cursor position
static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    
    // Cursor low port
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    
    // Cursor high port
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Create a VGA entry (character + color)
static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

// Terminal wrapper functions
void terminal_initialize(void) {
    vga_initialize();
}

void terminal_putchar(char c) {
    vga_put_char(c);
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
