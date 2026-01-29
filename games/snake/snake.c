#include "snake.h"
#include "../../kernel/vga_manager.h"
#include "../../drivers/keyboard/keyboard.h"

#define KEY_ESC 0x01

static void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}

void launch_snake() {
    vga_manager_clear();
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_manager_puts("Snake game started! Press ESC to exit.");

    uint32_t last_key = 0;
    while (1) {
        uint16_t scancode = keyboard_get_scancode();
        if (scancode != 0) {
            last_key = scancode;
        } else if (last_key != 0) {
            if (last_key == KEY_ESC) {
                vga_manager_clear();
                return;
            }
            last_key = 0;
        }
        delay(10000);
    }
}
