#include "vga_manager.h"
#include "kernel.h"

static vga_context_t kernel_context;
static vga_context_t gui_context;
static vga_context_t* current_context;

static int fullscreen_cursor_x = 0;
static int fullscreen_cursor_y = 0;
static uint8_t fullscreen_color = 0x0F;


static uint16_t* const vga_buffer = VGA_MEMORY;

static void update_cursor();
static uint16_t vga_entry(unsigned char uc, uint8_t color);
static void vga_manager_scroll();


void vga_manager_init(void) {
    kernel_context.cursor_x = 0;
    kernel_context.cursor_y = 0;
    kernel_context.color = 0x0F;
    kernel_context.width = 40;
    kernel_context.height = 25;
    kernel_context.start_x = 0;
    kernel_context.start_y = 0;

    gui_context.cursor_x = 0;
    gui_context.cursor_y = 0;
    gui_context.color = 0x0F;
    gui_context.width = 40;
    gui_context.height = 25;
    gui_context.start_x = 40;
    gui_context.start_y = 0;

    current_context = &kernel_context;

    vga_manager_set_context(false);
    vga_manager_clear();
    vga_manager_set_context(true);
    vga_manager_clear();

    vga_manager_set_context(false);
}

void vga_manager_set_context(bool is_gui) {
    current_context = is_gui ? &gui_context : &kernel_context;
    update_cursor();
}

void vga_manager_clear() {
    const uint16_t blank = vga_entry(' ', current_context->color);
    for (int y = 0; y < current_context->height; y++) {
        for (int x = 0; x < current_context->width; x++) {
            vga_buffer[(current_context->start_y + y) * VGA_WIDTH + current_context->start_x + x] = blank;
        }
    }
    current_context->cursor_x = 0;
    current_context->cursor_y = 0;
    update_cursor();
}

void vga_manager_set_color(enum vga_color fg, enum vga_color bg) {
    current_context->color = fg | (bg << 4);
}

void vga_manager_putchar(char c) {
    if (c == '\n') {
        current_context->cursor_x = 0;
        current_context->cursor_y++;
    } else if (c == '\r') {
        current_context->cursor_x = 0;
    } else if (c == '\b') {
        if (current_context->cursor_x > 0) {
            current_context->cursor_x--;
            vga_buffer[(current_context->start_y + current_context->cursor_y) * VGA_WIDTH + current_context->start_x + current_context->cursor_x] = vga_entry(' ', current_context->color);
        }
    } else if (c >= ' ') {
        vga_buffer[(current_context->start_y + current_context->cursor_y) * VGA_WIDTH + current_context->start_x + current_context->cursor_x] = vga_entry(c, current_context->color);
        current_context->cursor_x++;
    }

    if (current_context->cursor_x >= current_context->width) {
        current_context->cursor_x = 0;
        current_context->cursor_y++;
    }

    if (current_context->cursor_y >= current_context->height) {
        vga_manager_scroll();
        current_context->cursor_y = current_context->height - 1;
    }

    update_cursor();
}

void vga_manager_puts(const char* str) {
    while (*str) {
        vga_manager_putchar(*str++);
    }
}

static void vga_manager_scroll() {
    for (int y = 1; y < current_context->height; y++) {
        for (int x = 0; x < current_context->width; x++) {
            vga_buffer[(current_context->start_y + y - 1) * VGA_WIDTH + current_context->start_x + x] = vga_buffer[(current_context->start_y + y) * VGA_WIDTH + current_context->start_x + x];
        }
    }

    const uint16_t blank = vga_entry(' ', current_context->color);
    for (int x = 0; x < current_context->width; x++) {
        vga_buffer[(current_context->start_y + current_context->height - 1) * VGA_WIDTH + current_context->start_x + x] = blank;
    }
}

static void update_cursor(void) {
    uint16_t pos = (current_context->start_y + current_context->cursor_y) * VGA_WIDTH + current_context->start_x + current_context->cursor_x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

// Fullscreen functions
void vga_manager_fullscreen_clear(void) {
    const uint16_t blank = vga_entry(' ', fullscreen_color);
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
    fullscreen_cursor_x = 0;
    fullscreen_cursor_y = 0;
}

void vga_manager_fullscreen_set_color(enum vga_color fg, enum vga_color bg) {
    fullscreen_color = fg | (bg << 4);
}

void vga_manager_fullscreen_set_cursor_pos(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        fullscreen_cursor_x = x;
        fullscreen_cursor_y = y;
    }
}

void vga_manager_fullscreen_puts(const char* str) {
    while (*str) {
        vga_manager_fullscreen_putchar(*str++);
    }
}

void vga_manager_fullscreen_putchar(char c) {
    if (c == '\n') {
        fullscreen_cursor_x = 0;
        fullscreen_cursor_y++;
    } else if (c == '\r') {
        fullscreen_cursor_x = 0;
    } else if (c == '\b') {
        if (fullscreen_cursor_x > 0) {
            fullscreen_cursor_x--;
            vga_buffer[fullscreen_cursor_y * VGA_WIDTH + fullscreen_cursor_x] = vga_entry(' ', fullscreen_color);
        }
    } else if (c >= ' ') {
        vga_buffer[fullscreen_cursor_y * VGA_WIDTH + fullscreen_cursor_x] = vga_entry(c, fullscreen_color);
        fullscreen_cursor_x++;
    }

    if (fullscreen_cursor_x >= VGA_WIDTH) {
        fullscreen_cursor_x = 0;
        fullscreen_cursor_y++;
    }
}
