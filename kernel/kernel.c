#include "kernel.h"
#include "vga_manager.h"
#include "boot_anim.h"
#include "../gui/gui.h"
#include "shell.h"
#include "idt.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/ata/ata.h"
#include "fs.h"
#include "memory.h"
#include "timer.h"
#include "cpu.h"
#include "syscall.h"

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
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
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
    
    // Initialize CPU detection
    vga_manager_puts("Detecting CPU...\n");
    cpu_init();
    cpu_info_t* cpu = cpu_get_info();
    vga_manager_puts("CPU Vendor: ");
    vga_manager_puts(cpu->vendor);
    vga_manager_puts("\n");
    
    // Initialize memory manager
    vga_manager_puts("Initializing memory manager...\n");
    memory_init();
    
    // Initialize IDT and keyboard
    vga_manager_puts("Initializing IDT...\n");
    idt_init();
    
    vga_manager_puts("Initializing keyboard...\n");
    keyboard_install();
    
    vga_manager_puts("Initializing timer...\n");
    timer_init();
    
    vga_manager_puts("Initializing system calls...\n");
    syscall_init();
    
    // Enable interrupts
    asm volatile ("sti");
    
    // Initialize ATA driver
    vga_manager_puts("Initializing ATA driver...\n");
    ata_init();

    // ATA Driver Test
    vga_manager_puts("Running ATA driver test...\n");
    uint8_t write_buffer[512];
    uint8_t read_buffer[512];

    // Prepare a test pattern
    const char* test_string = "SkullOS ATA Driver Test! If you see this, writing and reading works.";
    for (uint16_t i = 0; i < 512; i++) {
        if (i < strlen(test_string)) {
            write_buffer[i] = test_string[i];
        } else {
            write_buffer[i] = 0;
        }
    }

    // Write to LBA 0
    vga_manager_puts("Writing to sector 0...\n");
    ata_write_sector(0, write_buffer);

    // Read from LBA 0
    vga_manager_puts("Reading from sector 0...\n");
    ata_read_sector(0, read_buffer);

    // Verify
    vga_manager_puts("Read back: \n");
    for(int i = 0; i < 512; i++) {
        if(read_buffer[i] != 0) {
            vga_manager_putchar(read_buffer[i]);
        }
    }
    vga_manager_puts("\nATA test complete.\n");
    
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
