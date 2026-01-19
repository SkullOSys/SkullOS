#include "keyboard.h"
#include "../../kernel/kernel.h"
#include "../../kernel/vga.h"
#include "../../kernel/pic.h"
#include <stddef.h>

// Current keyboard state
static uint8_t keyboard_modifiers = 0;
static bool keyboard_initialized = false;

// Keyboard buffer (circular buffer)
#define KEYBOARD_BUFFER_SIZE 128
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint32_t keyboard_buffer_start = 0;
static uint32_t keyboard_buffer_end = 0;

// Scancode set 1 to ASCII conversion table (US QWERTY)
static const char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Shifted scancode set 1 to ASCII conversion table (US QWERTY)
static const char kbdus_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Wait for the keyboard controller to be ready for writing
static void keyboard_wait(void) {
    uint8_t status;
    do {
        status = inb(KEYBOARD_STATUS_PORT);
    } while (status & 0x02); // Wait for input buffer to be clear
}

// Wait for the keyboard controller to have data ready to read
static void keyboard_wait_data(void) {
    uint8_t status;
    do {
        status = inb(KEYBOARD_STATUS_PORT);
    } while (!(status & 0x01)); // Wait for output buffer to have data
}

// Send a command to the keyboard controller
static void keyboard_send_command(uint8_t cmd) {
    keyboard_wait();
    outb(KEYBOARD_CMD_PORT, cmd);
}

// Send a command to the keyboard itself
static void keyboard_send_command_keyboard(uint8_t cmd) {
    keyboard_wait();
    outb(KEYBOARD_DATA_PORT, cmd);
}

// Read a byte from the keyboard controller
static uint8_t keyboard_read_data(void) {
    keyboard_wait_data();
    return inb(KEYBOARD_DATA_PORT);
}

// Handle a keyboard interrupt
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Handle key release (key up) events (0x80 bit set)
    if (scancode & 0x80) {
        uint8_t key = scancode & 0x7F;
        
        // Update modifier keys state
        switch (key) {
            case 0x1D: // Left Ctrl
                keyboard_modifiers &= ~MOD_CTRL;
                break;
            case 0x2A: // Left Shift
            case 0x36: // Right Shift
                keyboard_modifiers &= ~MOD_SHIFT;
                break;
            case 0x38: // Left Alt
                keyboard_modifiers &= ~MOD_ALT;
                break;
        }
    } else {
        // Handle key press (key down) events
        switch (scancode) {
            case 0x1D: // Left Ctrl
                keyboard_modifiers |= MOD_CTRL;
                break;
            case 0x2A: // Left Shift
            case 0x36: // Right Shift
                keyboard_modifiers |= MOD_SHIFT;
                break;
            case 0x38: // Left Alt
                keyboard_modifiers |= MOD_ALT;
                break;
            case 0x3A: // Caps Lock
                keyboard_modifiers ^= MOD_CAPS;
                break;
            case 0x45: // Num Lock
                keyboard_modifiers ^= MOD_NUM;
                break;
            case 0x46: // Scroll Lock
                keyboard_modifiers ^= MOD_SCROLL;
                break;
            default:
                // Add the scancode to the buffer if there's space
                if (((keyboard_buffer_end + 1) % KEYBOARD_BUFFER_SIZE) != keyboard_buffer_start) {
                    keyboard_buffer[keyboard_buffer_end] = scancode;
                    keyboard_buffer_end = (keyboard_buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
                }
                break;
        }
    }
    
    // Acknowledge the interrupt to the PIC
    pic_send_eoi(1);  // IRQ1 is the keyboard interrupt
}

// Convert a scancode to an ASCII character
char keyboard_scancode_to_ascii(uint8_t scancode) {
    if (scancode >= 128) {
        return 0; // No character for key release events
    }
    
    bool shift = (keyboard_modifiers & MOD_SHIFT) || 
                ((keyboard_modifiers & MOD_CAPS) && (scancode >= 0x10 && scancode <= 0x1A));
    
    if (shift) {
        return kbdus_shift[scancode];
    } else {
        return kbdus[scancode];
    }
}

// Check if a key is currently pressed
bool keyboard_is_key_pressed(uint8_t scancode) {
    // This is a simplified version - in a real implementation, you'd track the state of each key
    // For now, we'll just check if the key is in the buffer
    for (uint32_t i = keyboard_buffer_start; i != keyboard_buffer_end; i = (i + 1) % KEYBOARD_BUFFER_SIZE) {
        if (keyboard_buffer[i] == scancode) {
            return true;
        }
    }
    return false;
}

// Get a character from the keyboard buffer (blocking)
char keyboard_getchar(void) {
    while (keyboard_buffer_start == keyboard_buffer_end) {
        // Wait for a key to be pressed
        asm volatile ("hlt");
    }
    
    uint8_t scancode = keyboard_buffer[keyboard_buffer_start];
    keyboard_buffer_start = (keyboard_buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
    
    return keyboard_scancode_to_ascii(scancode);
}

// Get a scancode from the keyboard buffer (non-blocking)
uint16_t keyboard_get_scancode(void) {
    if (keyboard_buffer_start == keyboard_buffer_end) {
        return 0;
    }
    
    uint8_t scancode = keyboard_buffer[keyboard_buffer_start];
    keyboard_buffer_start = (keyboard_buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
    
    return scancode;
}

void keyboard_reset(void) {
    keyboard_initialized = false;
}

// Initialize the keyboard controller
void keyboard_init(void) {
    if (keyboard_initialized) {
        return;
    }
    
    // Disable interrupts
    asm volatile ("cli");
    
    // Clear the keyboard buffer
    keyboard_buffer_start = 0;
    keyboard_buffer_end = 0;
    
    // Enable the first PS/2 port
    keyboard_send_command(0xAE);
    
    // Get the current controller configuration byte
    keyboard_send_command(0x20);
    uint8_t config = keyboard_read_data();
    
    // Enable keyboard interrupt (bit 0)
    config |= 0x01;
    
    // Write the new configuration byte back
    keyboard_send_command(0x60);
    keyboard_send_command_keyboard(config);
    
    // Enable interrupts
    asm volatile ("sti");
    
    keyboard_initialized = true;
}

// Install the keyboard interrupt handler
void keyboard_install(void) {
    // The actual IRQ1 (keyboard) handler installation would go here
    // This is typically done by setting up the IDT (Interrupt Descriptor Table)
    // For now, we'll just initialize the keyboard
    keyboard_init();
}
