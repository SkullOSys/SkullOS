#ifndef BIOS_H
#define BIOS_H

#include <stdint.h>
#include <stdbool.h>
#include "../drivers/keyboard/keyboard.h"
#include "../kernel/vga_manager.h"

// E820 memory map entry structure
typedef struct __attribute__((packed)) {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_ext;
} e820_entry_t;

// E820 memory types
#define E820_RAM        1   // Usable memory
#define E820_RESERVED   2   // Reserved memory
#define E820_ACPI       3   // ACPI Reclaimable memory
#define E820_NVS        4   // ACPI NVS memory
#define E820_BADRAM     5   // Bad memory

#define KEY_UP_ARROW 0x48
#define KEY_DOWN_ARROW 0x50
#define KEY_ENTER 0x1C

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
