#include "kernel.h"
#include "vga.h"
#include "boot_anim.h"
#include "../gui/gui.h"

// Kernel entry point
__attribute__((section(".text.entry")))
void kernel_entry(void) {
    // Initialize VGA text mode
    terminal_initialize();
    
    // Call the main kernel function
    kernel_main();
    
    // Halt if kernel_main returns (shouldn't happen)
    panic("kernel_main returned!");
}

// Main kernel function
void kernel_main(void) {
    // Initialize terminal and show boot animation
    terminal_initialize();
    show_boot_animation();
    
    // Set default color to white on black
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    // Display system information
    terminal_puts("\n====== SkullOS System Information ======\n\n");
    
    // Show kernel information
    terminal_puts("Kernel version: 0.1.0\n");
    terminal_puts("Build date: ");
    terminal_puts(__DATE__);
    terminal_puts(" ");
    terminal_puts(__TIME__);
    terminal_puts("\n\n");
    
    // Initialize GUI
    terminal_puts("Initializing system components...\n");
    gui_init();
    terminal_puts("GUI initialized.\n\n");
    
    // Disable cursor
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
    
    // Main kernel loop
    while (1) {
        // We'll add command processing here later
        asm volatile ("hlt");
    }
}
