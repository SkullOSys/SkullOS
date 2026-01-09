#include "shell.h"
#include "vga.h"
#include "kernel.h"

#define MAX_COMMAND_LENGTH 80

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_length = 0;

// Command handlers
typedef void (*command_handler_t)(void);

typedef struct {
    const char* name;
    const char* description;
    command_handler_t handler;
} command_t;

// Forward declarations of command handlers
static void cmd_help(void);
static void cmd_clear(void);

// List of supported commands
static const command_t commands[] = {
    {"help", "Show this help message", cmd_help},
    {"clear", "Clear the screen", cmd_clear},
    {NULL, NULL, NULL}  // End marker
};

// Command implementations
static void cmd_help(void) {
    terminal_puts("\nAvailable commands:\n");
    
    const command_t* cmd = commands;
    while (cmd->name) {
        terminal_puts("  ");
        terminal_puts(cmd->name);
        // Pad to align descriptions
        int padding = 12 - strlen(cmd->name);
        for (int i = 0; i < padding; i++) {
            terminal_puts(" ");
        }
        terminal_puts("- ");
        terminal_puts(cmd->description);
        terminal_puts("\n");
        cmd++;
    }
    terminal_puts("\n");
}

static void cmd_clear(void) {
    vga_clear_screen();
    shell_print_prompt();
}

void shell_init(void) {
    command_length = 0;
    memset(command_buffer, 0, sizeof(command_buffer));
}

void shell_print_prompt(void) {
    terminal_puts("skullos> ");
}

void shell_process_command(const char* command) {
    if (strlen(command) == 0) {
        return;  // Empty command
    }
    
    // Find the command
    const command_t* cmd = commands;
    while (cmd->name) {
        if (strcmp(command, cmd->name) == 0) {
            cmd->handler();
            return;
        }
        cmd++;
    }
    
    // Command not found
    terminal_puts("Unknown command: ");
    terminal_puts(command);
    terminal_puts("\nType 'help' for a list of commands.\n");
}

void shell_run(void) {
    shell_print_prompt();
    
    while (1) {
        char c = 0;
        // Wait for a key press
        asm volatile("hlt");
        
        // In a real implementation, you would read from the keyboard here
        // For now, we'll just handle Enter and Backspace
        // This is a placeholder - you'll need to implement proper keyboard input
        
        if (c == '\r' || c == '\n') {
            // Execute command
            terminal_puts("\n");
            command_buffer[command_length] = '\0';
            shell_process_command(command_buffer);
            
            // Reset for next command
            command_length = 0;
            memset(command_buffer, 0, sizeof(command_buffer));
            shell_print_prompt();
        } else if (c == '\b' && command_length > 0) {
            // Backspace
            command_length--;
            command_buffer[command_length] = '\0';
            // Move cursor back, print space, move cursor back again
            terminal_puts("\b \b");
        } else if (command_length < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
            // Printable character
            command_buffer[command_length++] = c;
            terminal_putchar(c);
        }
    }
}
