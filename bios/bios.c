#include "bios.h"
#include "../drivers/keyboard/keyboard.h"
#include "../kernel/vga_manager.h"
#include <stddef.h>

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
    // Initialize any BIOS-specific hardware here
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
    char *p = mem_str + sizeof(mem_str) - 1;
    *p = '\0';
    uint32_t n = bios_config.memory_size_mb;
    do {
        *--p = '0' + (n % 10);
        n /= 10;
    } while (n > 0);
    vga_manager_puts(p);
    vga_manager_puts(" MB\n\n");
    
    vga_manager_puts("\n\nPress ESC to exit BIOS...");
    
    // Wait for ESC key with debounce
    uint32_t last_key = 0;
    while (1) {
        uint16_t scancode = keyboard_get_scancode();
        if (scancode != 0) {
            last_key = scancode;
        } else if (last_key == 0x01) {  // ESC key released
            delay(100000);  // Small delay for debounce
            break;
        }
        delay(10000);  // Small delay to prevent CPU spin
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
    // In a real implementation, this would save to NVRAM or disk
    // For now, we just keep it in memory
}
