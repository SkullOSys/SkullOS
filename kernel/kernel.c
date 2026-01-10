#include "kernel.h"
#include "vga.h"
#include "boot_anim.h"
#include "../gui/gui.h"
#include "shell.h"
#include "idt.h"
#include "../drivers/keyboard/keyboard.h"
#include "fs.h"
#include "memory.h"

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
    
    // Initialize memory manager
    terminal_puts("Initializing memory manager...\n");
    memory_init();
    
    // Initialize IDT and keyboard
    terminal_puts("Initializing IDT...\n");
    idt_init();
    
    terminal_puts("Initializing keyboard...\n");
    keyboard_install();
    
    // Enable interrupts
    asm volatile ("sti");
    
    // Initialize filesystem
    terminal_puts("Initializing filesystem...\n");
    fs_initialize();
    
    // Initialize and run the shell
    terminal_puts("Initializing shell...\n");
    shell_init();
    fs_init_commands();  // Register filesystem commands
    terminal_puts("\nType 'help' for a list of available commands.\n\n");
    shell_run();
    
    // This should never be reached
    while (1) {
        asm volatile ("hlt");
    }
}
