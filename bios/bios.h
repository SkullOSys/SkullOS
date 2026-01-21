#ifndef BIOS_H
#define BIOS_H

#include <stdint.h>
#include <stdbool.h>
#include "../drivers/keyboard/keyboard.h"
#include "../kernel/vga_manager.h"

// BIOS configuration structure
typedef struct {
    bool show_fps;
    bool debug_mode;
    uint8_t boot_device;
    uint32_t memory_size_mb;
} bios_config_t;

// Initialize BIOS
void bios_init(void);

// Show BIOS interface
void bios_show_interface(void);

// Check if BIOS key was pressed (spacebar during boot)
bool bios_check_key(void);

// Get BIOS configuration
bios_config_t* bios_get_config(void);

// Save BIOS configuration
void bios_save_config(void);

#endif // BIOS_H
