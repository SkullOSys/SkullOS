#include "kernel.h"
#include "vga_manager.h"
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
    vga_manager_init();
    
    // Call the main kernel function
    kernel_main();
    
    // Halt if kernel_main returns (shouldn't happen)
    panic("kernel_main returned!");
}

// Main kernel function
void kernel_main(void) {
    // Initialize terminal and show boot animation
    show_boot_animation();
    
    // Set default color to white on black
    vga_manager_set_context(false);
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    // Display system information
    vga_manager_puts("\n====== SkullOS System Information ======\n\n");
    
    // Show kernel information
    vga_manager_puts("Kernel version: 0.1.0\n");
    vga_manager_puts("Build date: ");
    vga_manager_puts(__DATE__);
    vga_manager_puts(" ");
    vga_manager_puts(__TIME__);
    vga_manager_puts("\n\n");
    
    // Initialize GUI
    vga_manager_puts("Initializing system components...\n");
    gui_init();
    vga_manager_set_context(true);
    const char* msg = "Skull GUI Initialized";
    int msg_len = strlen(msg);
    int x = (40 - msg_len) / 2;
    for (int i = 0; i < x; i++) {
        vga_manager_putchar(' ');
    }
    vga_manager_puts(msg);
    vga_manager_set_context(false);
    vga_manager_puts("GUI initialized.\n\n");
    
    // Disable cursor
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
    
    // Initialize memory manager
    vga_manager_puts("Initializing memory manager...\n");
    memory_init();
    
    // Initialize IDT and keyboard
    vga_manager_puts("Initializing IDT...\n");
    idt_init();
    
    vga_manager_puts("Initializing keyboard...\n");
    keyboard_install();
    
    // Enable interrupts
    asm volatile ("sti");
    
    // Initialize filesystem
    vga_manager_puts("Initializing filesystem...\n");
    fs_initialize();
    
    // Initialize and run the shell
    vga_manager_puts("Initializing shell...\n");
    shell_init();
    fs_init_commands();  // Register filesystem commands
    vga_manager_puts("\nType 'help' for a list of available commands.\n\n");
    shell_run();
    
    // This should never be reached
    while (1) {
        asm volatile ("hlt");
    }
}
