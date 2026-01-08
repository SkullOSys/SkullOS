#include "gui.h"

// VGA Text mode address
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Function to display a message at a specific position
static void gui_put_string_at(const char* str, int x, int y) {
    volatile unsigned short* vga = (volatile unsigned short*)VGA_ADDRESS;
    int pos = y * VGA_WIDTH + x;
    
    for (int i = 0; str[i] != '\0'; i++) {
        vga[pos + i] = (unsigned short)str[i] | (0x0F << 8);
    }
}

// Function to initialize the GUI
void gui_init() {
    // Display initialization message in the middle of the screen
    const char* msg = "GUI Initialized";
    int msg_len = 0;
    
    // Calculate message length
    while (msg[msg_len] != '\0') {
        msg_len++;
    }
    
    // Calculate position to center the message
    int x = (VGA_WIDTH - msg_len) / 2;
    int y = VGA_HEIGHT / 2;
    
    // Display the message
    gui_put_string_at(msg, x, y);
}