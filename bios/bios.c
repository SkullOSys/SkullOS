#include "bios.h"
#include "../drivers/keyboard/keyboard.h"
#include "../kernel/vga_manager.h"
#include <stddef.h>
#include <libc/include/string.h>
#include "../fs/include/fs.h"

// Simple delay function
static void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}

// BIOS configuration in memory
static bios_config_t bios_config = {
    .show_fps = false,
    .debug_mode = false,
    .boot_device = 0x80,  // Default to first hard drive
    .memory_size_mb = 0   // Will be detected
};

void bios_init(void) {
    // Detect memory from e820 map provided by bootloader
    uint32_t num_entries = *(uint32_t*)0x8500;
    e820_entry_t* mem_map = (e820_entry_t*)0x8504;
    
    uint64_t total_mem = 0;
    
    if (num_entries > 0) {
        for (uint32_t i = 0; i < num_entries; i++) {
            if (mem_map[i].type == E820_RAM) {
                total_mem += mem_map[i].length;
            }
        }
    }
    
    bios_config.memory_size_mb = total_mem / (1024 * 1024);

    // Load config from file
    fs_node_t *node = finddir_fs(fs_root, "bios.cfg");
    if (node) {
        read_fs(node, 0, sizeof(bios_config_t), (uint8_t*)&bios_config);
    }
}

static void bios_system_config_menu(void) {
    int selected_item = 0;
    const int menu_items = 2;

    while (1) {
        vga_manager_clear();
        vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
        vga_manager_puts(" System Configuration ");
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
                    vga_manager_puts(" Toggle Show FPS (Currently: ");
                    vga_manager_puts(bios_config.show_fps ? "Enabled" : "Disabled");
                    vga_manager_puts(")\n");
                    break;
                case 1:
                    vga_manager_puts(" Toggle Debug Mode (Currently: ");
                    vga_manager_puts(bios_config.debug_mode ? "Enabled" : "Disabled");
                    vga_manager_puts(")\n");
                    break;
            }
            vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
        vga_manager_puts("\nESC. Back to main menu\n\n");

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
                        bios_config.show_fps = !bios_config.show_fps;
                    } else if (selected_item == 1) {
                        bios_config.debug_mode = !bios_config.debug_mode;
                    }
                } else if (last_key == 0x01) { // ESC key
                    delay(100000);
                    return;
                }
                last_key = 0;
                break; // Re-draw the menu
            }
            delay(10000);
        }
    }
}

void bios_show_interface(void) {
    // Save current VGA context
    bool prev_context = vga_manager_get_context();
    vga_manager_set_context(false);

    int selected_item = 0;
    const int menu_items = 4;

    while (1) {
        // Clear screen
        vga_manager_clear();

        // Draw BIOS header
        vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
        vga_manager_puts(" SkullOS BIOS v1.0 - Setup Utility ");
        vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_manager_puts("\n\n");

        // Show menu
        for (int i = 0; i < menu_items; i++) {
            if (i == selected_item) {
                vga_manager_set_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
                vga_manager_putchar('>');
            } else {
                vga_manager_putchar(' ');
            }
            switch (i) {
                case 0: vga_manager_puts(" 1. Boot Options\n"); break;
                case 1: vga_manager_puts(" 2. System Configuration\n"); break;
                case 2: vga_manager_puts(" 3. Power Management\n"); break;
                case 3: vga_manager_puts(" 4. Save & Exit\n\n"); break;
            }
            vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }

        // Show current configuration
        vga_manager_puts("Current Configuration:\n");
        vga_manager_puts("  Show FPS: ");
        vga_manager_puts(bios_config.show_fps ? "Enabled" : "Disabled");
        vga_manager_puts("\n  Debug Mode: ");
        vga_manager_puts(bios_config.debug_mode ? "Enabled" : "Disabled");
        vga_manager_puts("\n  Boot Device: ");
        vga_manager_puts(bios_config.boot_device == 0x80 ? "HDD" : "Other");
        vga_manager_puts("\n  Memory: ");

        // Simple itoa for memory size
        char mem_str[16];
        itoa(bios_config.memory_size_mb, mem_str, 10);
        vga_manager_puts(mem_str);
        vga_manager_puts(" MB\n\n");

        vga_manager_puts("\n\nPress a key to select an option...");
        vga_manager_puts("\nPress a ESC to exit BIOS...");

        // Wait for key with debounce
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
                    if (selected_item == 1) { // System Configuration
                        bios_system_config_menu();
                    } else if (selected_item == 3) { // Save & Exit
                        bios_save_config();
                        delay(100000);
                        goto exit_loop;
                    }
                } else if (last_key == 0x01) {  // ESC key to exit without saving
                    delay(100000);
                    goto exit_loop;
                }
                last_key = 0;
                break;  // Re-draw the menu
            }
            delay(10000);
        }
    }

exit_loop:
    // Restore VGA context
    vga_manager_set_context(prev_context);
}

bool bios_check_key(void) {
    // Check if spacebar is pressed
    uint16_t scancode = keyboard_get_scancode();
    return (scancode == 0x39);  // Spacebar scan code
}

bios_config_t* bios_get_config(void) {
    return &bios_config;
}

void bios_save_config(void) {
    fs_node_t *node = finddir_fs(fs_root, "bios.cfg");
    if (!node) {
        node = skullfs_create_file(fs_root, "bios.cfg");
    }
    write_fs(node, 0, sizeof(bios_config_t), (uint8_t*)&bios_config);
}
