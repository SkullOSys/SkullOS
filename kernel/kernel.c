#include "kernel.h"
#include "vga.h"
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
    // Clear the screen and set color to white on black
    terminal_initialize();
    
    // Print welcome message
    terminal_puts("\n  ===================================\n");
    terminal_puts("            Skull OS             \n");
    terminal_puts("  ===================================\n\n");
    
    // Display some system information
    terminal_puts("Kernel initialized successfully!\n");
    terminal_puts("System ready.\n\n");
    
    // Test some terminal functions
    terminal_puts("Testing terminal functions:\n");
    
    // Test colors
    vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
    terminal_puts("Red text\n");
    
    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    terminal_puts("Green text\n");
    
    vga_set_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    terminal_puts("Blue text\n");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_puts("\nNumbers: ");
    
    // Test number printing
    for (int i = 1; i <= 5; i++) {
        terminal_put_dec(i);
        terminal_puts(" ");
    }
    
    // Test hex output
    terminal_puts("\n\nMemory address of kernel_main: 0x");
    terminal_put_hex((uint32_t)&kernel_main);
    terminal_puts("\n");
    
    // Initialize GUI (if available)
    terminal_puts("\nInitializing GUI...\n");
    gui_init();
    
    // Main kernel loop
    terminal_puts("\nKernel is running. System ready.\n");
    
    // Infinite loop to keep the kernel running
    while (1) {
        // We'll add more functionality here later
        asm volatile ("hlt");
    }
}
