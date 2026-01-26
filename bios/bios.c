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
    vga_manager_clear();
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_puts(" System Configuration ");
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_manager_puts("\n\n");
    vga_manager_puts("1. Toggle Show FPS (Currently: ");
    vga_manager_puts(bios_config.show_fps ? "Enabled" : "Disabled");
    vga_manager_puts(")\n");
    vga_manager_puts("2. Toggle Debug Mode (Currently: ");
    vga_manager_puts(bios_config.debug_mode ? "Enabled" : "Disabled");
    vga_manager_puts(")\n");
    vga_manager_puts("ESC. Back to main menu\n\n");

    uint32_t last_key = 0;
    while (1) {
        uint16_t scancode = keyboard_get_scancode();
        if (scancode != 0) {
            last_key = scancode;
        } else if (last_key != 0) {
            if (last_key == 0x02) { // '1' key
                bios_config.show_fps = !bios_config.show_fps;
                // Redraw menu
                bios_system_config_menu();
                return;
            } else if (last_key == 0x03) { // '2' key
                bios_config.debug_mode = !bios_config.debug_mode;
                // Redraw menu
                bios_system_config_menu();
                return;
            } else if (last_key == 0x01) { // ESC key
                delay(100000);
                return;
            }
            last_key = 0;
        }
        delay(10000);
    }
}

void bios_show_interface(void) {
    // Save current VGA context
    bool prev_context = vga_manager_get_context();
    vga_manager_set_context(false);
    
    // Clear screen
    vga_manager_clear();
    
    // Draw BIOS header
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_puts(" SkullOS BIOS v1.0 - Setup Utility ");
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_manager_puts("\n\n");
    
    // Show menu
    vga_manager_puts("1. Boot Options\n");
    vga_manager_puts("2. System Configuration\n");
    vga_manager_puts("3. Power Management\n");
    vga_manager_puts("4. Save & Exit\n\n");
    
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
    
    vga_manager_puts("\n\nPress a ESC to exit BIOS...");
    vga_manager_puts("\n\nPress a key to select an option...");
    
    // Wait for key with debounce
    uint32_t last_key = 0;
    while (1) {
        uint16_t scancode = keyboard_get_scancode();
        if (scancode != 0) {
            last_key = scancode;
        } else if (last_key != 0) {
            if (last_key == 0x03) { // '2' key for System Configuration
                bios_system_config_menu();
                // Redraw main menu after returning
                bios_show_interface();
                return;
            } else if (last_key == 0x05) { // '4' key for Save & Exit
                bios_save_config();
                delay(100000);
                break;
            } else if (last_key == 0x01) {  // ESC key to exit without saving
                delay(100000);
                break;
            }
            last_key = 0;
        }
        delay(10000);
    }
    
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
