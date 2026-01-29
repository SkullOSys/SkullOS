#include "games.h"
#include "snake/snake.h"
#include "../kernel/vga_manager.h"
#include "../drivers/keyboard/keyboard.h"
#include <stddef.h>

#define KEY_UP_ARROW 0x48
#define KEY_DOWN_ARROW 0x50
#define KEY_ENTER 0x1C
#define KEY_ESC 0x01

static void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}

void launch_games() {
    int selected_item = 0;
    const int menu_items = 1;

    while (1) {
        vga_manager_clear();
        vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
        vga_manager_puts(" Games ");
        vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_manager_puts("\n\n");

        for (int i = 0; i < menu_items; i++) {
            if (i == selected_item) {
                vga_manager_set_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
                vga_manager_putchar('>');
            } else {
                vga_manager_putchar(' ');
            }
            switch (i) {
                case 0:
                    vga_manager_puts(" Snake\n");
                    break;
            }
            vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
        vga_manager_puts("\nESC. Back to shell\n\n");

        uint32_t last_key = 0;
        while (1) {
            uint16_t scancode = keyboard_get_scancode();
            if (scancode != 0) {
                last_key = scancode;
            } else if (last_key != 0) {
                if (last_key == KEY_DOWN_ARROW) {
                    selected_item = (selected_item + 1) % menu_items;
                } else if (last_key == KEY_UP_ARROW) {
                    selected_item = (selected_item - 1 + menu_items) % menu_items;
                } else if (last_key == KEY_ENTER) {
                    if (selected_item == 0) {
                        // Launch Snake
                        launch_snake();
                    }
                } else if (last_key == KEY_ESC) {
                    delay(100000);
                    vga_manager_clear();
                    return;
                }
                last_key = 0;
                break; // Re-draw the menu
            }
            delay(10000);
        }
    }
}
