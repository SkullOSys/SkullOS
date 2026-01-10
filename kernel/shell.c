#include "shell.h"
#include "vga.h"
#include "kernel.h"
#include "../drivers/keyboard/keyboard.h"
#include "libc/include/string.h"

#define MAX_COMMAND_LENGTH 80

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_length = 0;

// Command entry structure
typedef struct command_t {
    const char* name;
    const char* description;
    command_handler_t handler;
    struct command_t* next;
} command_t;

// Head of the command list
static command_t* command_list = NULL;

// Forward declarations of command handlers
static void cmd_help(int argc, char **argv);
static void cmd_clear(int argc, char **argv);



// Command implementations
static void cmd_help(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    vga_put_string("\nAvailable commands:\n");
    
    command_t* cmd = command_list;
    while (cmd) {
        vga_put_string("  ");
        vga_put_string(cmd->name);
        // Pad to align descriptions
        int padding = 12 - strlen(cmd->name);
        for (int i = 0; i < padding; i++) {
            vga_put_string(" ");
        }
        vga_put_string("- ");
        vga_put_string(cmd->description);
        vga_put_string("\n");
        cmd = cmd->next;
    }
    vga_put_string("\n");
}

static void cmd_clear(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
vga_clear_screen();
}

// Register a new command
void shell_register_command(const char* name, const char* description, command_handler_t handler) {
    command_t* new_cmd = (command_t*)kmalloc(sizeof(command_t));
    if (!new_cmd) return;
    
    new_cmd->name = name;
    new_cmd->description = description;
    new_cmd->handler = handler;
    new_cmd->next = command_list;
    command_list = new_cmd;
}

// Parse command line into arguments
int shell_parse_arguments(char *line, char **argv, int max_args) {
    int argc = 0;
    char *p = line;
    
    while (*p && argc < max_args - 1) {
        // Skip leading whitespace
        while (*p == ' ' || *p == '\t') {
            *p++ = '\0';
        }
        
        if (!*p) break;
        
        // Start of a new argument
        argv[argc++] = p;
        
        // Skip to next whitespace or end of string
        while (*p && *p != ' ' && *p != '\t') {
            p++;
        }
    }
    
    argv[argc] = NULL;
    return argc;
}

// Initialize the shell
void shell_init(void) {
    command_length = 0;
    command_buffer[0] = '\0';
    command_list = NULL;
    
    // Register built-in commands
    shell_register_command("help", "Show this help message", cmd_help);
    shell_register_command("clear", "Clear the screen", cmd_clear);
}

void shell_print_prompt(void) {
    vga_put_string("skullos> ");
}

// Process a command
void shell_process_command(const char* command) {
    // Make a copy of the command line for parsing
    char cmd_line[MAX_COMMAND_LENGTH];
    strncpy(cmd_line, command, MAX_COMMAND_LENGTH - 1);
    cmd_line[MAX_COMMAND_LENGTH - 1] = '\0';
    
    // Parse arguments
    char* argv[16];  // Max 16 arguments
    int argc = shell_parse_arguments(cmd_line, argv, 16);
    
    if (argc == 0) {
        return;  // Empty command
    }
    
    // Echo the command
    vga_put_string("\n");
    
    // Look for the command
    command_t* cmd = command_list;
    while (cmd) {
        if (strcmp(argv[0], cmd->name) == 0) {
            cmd->handler(argc, argv);
            return;
        }
        cmd = cmd->next;
    }
    
    // Command not found
    vga_put_string("Command not found: ");
    vga_put_string(argv[0]);
    vga_put_string("\nType 'help' for a list of available commands.\n");
}

void shell_run(void) {
    shell_print_prompt();
    
    while (1) {
        // Get a character from the keyboard
        char c = keyboard_getchar();
        
        // Handle special keys
        if (c == '\r' || c == '\n') {
            // Execute command
            vga_put_string("\n");
            command_buffer[command_length] = '\0';
            shell_process_command(command_buffer);
            
            // Reset for next command
            command_length = 0;
            memset(command_buffer, 0, sizeof(command_buffer));
            shell_print_prompt();
        } else if (c == '\b' || c == 127) {  // Backspace or Delete
            if (command_length > 0) {
                command_length--;
                command_buffer[command_length] = '\0';
                // Move cursor back, print space, move cursor back again
                vga_put_string("\b \b");
            }
        } else if (command_length < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
            // Printable character
            command_buffer[command_length++] = c;
            vga_put_char(c);
        }
    }
}
