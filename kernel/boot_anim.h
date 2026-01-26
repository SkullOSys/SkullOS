#ifndef BOOT_ANIM_H
#define BOOT_ANIM_H

#include "vga_manager.h"
#include <string.h>

// ASCII Art Skull - New design
static const char* SKULL_ART[] = {
    "                 _________-----_____",
    "        _____-----           __      -----_",
    "  ___----             ___------              \\\\",
    "     ----________        ----                 \\\\",
    "                 -----__    |             _____)",
    "                      __-                /     \\\\",
    "          _______-----    ___--          \\\\    /)\\\\  ",
    "    ------_______      ---____            \\\\__/  /",
    "                 -----__    \\\\ --    _          /\\\\",
    "                        --__--__     \\\\_____/   \\\\_/\\\\",
    "                                ----|   /          |",
    "                                    |  |___________|",
    "                                    |  | ((_(_)| )_)",
    "                                    |  \\\\_((_(_)|/(_)",
    "                                    \\\\             (",
    "                                     \\\\_____________)",
    "",
    "                                Welcome to SkullOS"
};

#define SKULL_LINES (sizeof(SKULL_ART) / sizeof(char*))
#define LOADING_BAR_WIDTH 60



// Function to display the boot animation
void show_boot_animation(void) {
    // Clear screen and set color to bright white on black for better visibility
    vga_manager_fullscreen_clear();
    vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Center the skull vertically
    int start_y = (VGA_HEIGHT - SKULL_LINES - 6) / 2;
    if (start_y < 2) start_y = 2;
    
    // Print the skull art
    for (size_t i = 0; i < SKULL_LINES; i++) {
        int x = (VGA_WIDTH - strlen(SKULL_ART[i])) / 2;
        vga_manager_fullscreen_set_cursor_pos(x, start_y + i);
        vga_manager_fullscreen_puts(SKULL_ART[i]);
    }
    
    // Print loading bar frame
    int bar_x = (VGA_WIDTH - LOADING_BAR_WIDTH) / 2;
    int bar_y = start_y + SKULL_LINES + 2;
    
    vga_manager_fullscreen_set_cursor_pos(bar_x - 2, bar_y - 1);
    vga_manager_fullscreen_puts("[");
    for (int i = 0; i < LOADING_BAR_WIDTH; i++) {
        vga_manager_fullscreen_putchar(' ');
    }
    vga_manager_fullscreen_puts("]");
    
    // Animate loading bar
    for (int progress = 0; progress <= 100; progress++) {
        // Calculate number of filled characters
        int filled = (progress * LOADING_BAR_WIDTH) / 100;
        
        // Update loading bar
        vga_manager_fullscreen_set_cursor_pos(bar_x, bar_y - 1);
        vga_manager_fullscreen_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
        for (int i = 0; i < filled; i++) {
            vga_manager_fullscreen_putchar('#');
        }
        
        // Show percentage with better visibility
        vga_manager_fullscreen_set_cursor_pos(bar_x + LOADING_BAR_WIDTH + 3, bar_y - 1);
        vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_manager_fullscreen_puts("[");
        
        if (progress < 10) vga_manager_fullscreen_putchar(' ');
        if (progress < 100) vga_manager_fullscreen_putchar(' ');
        
        char percent[5];
        itoa(progress, percent, 10);
        vga_manager_fullscreen_puts(percent);
        vga_manager_fullscreen_puts("%]");
        
        // Simulate work being done (slowed down to make animation more visible)
        for (volatile int i = 0; i < 5000000; i++);
    }
    
    // Show completion message with better visibility
    vga_manager_fullscreen_set_cursor_pos((VGA_WIDTH - 25) / 2, bar_y + 2);
    vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_manager_fullscreen_puts("System ready! Booting...");
    
    // Wait for 10 seconds to show the completion
    for (volatile long i = 0; i < 1000000000; i++);
    
    // Clear the screen for the actual OS
    vga_manager_fullscreen_clear();
}

#endif // BOOT_ANIM_H
