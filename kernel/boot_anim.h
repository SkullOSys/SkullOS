#ifndef BOOT_ANIM_H
#define BOOT_ANIM_H

#include "vga.h"

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

// Simple itoa implementation (helper function for integers to string)
static char* itoa(int value, char* str, int base) {
    (void)base; // Base parameter is not used in this implementation
    
    char* ptr = str, *ptr1 = str, tmp_char;

    // Handle 0 explicitly, otherwise empty string is printed
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return str;
    }

    // Process individual digits
    while (value != 0) {
        *ptr++ = '0' + (value % 10);
        value /= 10;
    }
    *ptr = '\0';

    // Reverse the string
    ptr--;
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return str;
}

// Function to display the boot animation
void show_boot_animation(void) {
    // Clear screen and set color to bright white on black for better visibility
    vga_clear_screen();
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Center the skull vertically
    int start_y = (VGA_HEIGHT - SKULL_LINES - 6) / 2;
    if (start_y < 2) start_y = 2;
    
    // Print the skull art
    for (int i = 0; i < SKULL_LINES; i++) {
        int x = (VGA_WIDTH - strlen(SKULL_ART[i])) / 2;
        vga_set_cursor_pos(x, start_y + i);
        vga_put_string(SKULL_ART[i]);
    }
    
    // Print loading bar frame
    int bar_x = (VGA_WIDTH - LOADING_BAR_WIDTH) / 2;
    int bar_y = start_y + SKULL_LINES + 2;
    
    vga_set_cursor_pos(bar_x - 2, bar_y - 1);
    vga_put_string("[");
    for (int i = 0; i < LOADING_BAR_WIDTH; i++) {
        vga_put_char(' ');
    }
    vga_put_string("]");
    
    // Animate loading bar
    for (int progress = 0; progress <= 100; progress++) {
        // Calculate number of filled characters
        int filled = (progress * LOADING_BAR_WIDTH) / 100;
        
        // Update loading bar
        vga_set_cursor_pos(bar_x, bar_y - 1);
        vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
        for (int i = 0; i < filled; i++) {
            vga_put_char('#');
        }
        
        // Show percentage with better visibility
        vga_set_cursor_pos(bar_x + LOADING_BAR_WIDTH + 3, bar_y - 1);
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_put_string("[");
        
        if (progress < 10) vga_put_char(' ');
        if (progress < 100) vga_put_char(' ');
        
        char percent[5];
        itoa(progress, percent, 10);
        vga_put_string(percent);
        vga_put_string("%]");
        
        // Simulate work being done (slowed down to make animation more visible)
        for (volatile int i = 0; i < 5000000; i++);
    }
    
    // Show completion message with better visibility
    vga_set_cursor_pos((VGA_WIDTH - 25) / 2, bar_y + 2);
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_put_string("System ready! Booting...");
    
    // Wait for 10 seconds to show the completion
    for (volatile long i = 0; i < 1000000000; i++);
    
    // Clear the screen for the actual OS
    vga_clear_screen();
}

#endif // BOOT_ANIM_H
