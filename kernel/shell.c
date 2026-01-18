#include "shell.h"
#include "terminal.h"
#include "kernel.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/rtc/rtc.h"
#include "libc/include/string.h"
#include "vga_manager.h"
#include "timer.h"
#include "cpu.h"
#include "memory.h"
#include "util.h"

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
static void cmd_info(int argc, char **argv);



// Command implementations
static void cmd_help(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    terminal_puts("\nAvailable commands:\n");
    
    command_t* cmd = command_list;
    while (cmd) {
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
        cmd = cmd->next;
    }
    terminal_puts("\n");
}

static void cmd_clear(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    vga_manager_set_context(false);
    vga_manager_clear();
}

static void cmd_info(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    terminal_puts("\n=== SkullOS System Information ===\n\n");
    
    // CPU Info
    cpu_info_t* cpu = cpu_get_info();
    terminal_puts("CPU Vendor: ");
    terminal_puts(cpu->vendor);
    terminal_puts("\n");
    
    // Memory Info
    size_t free_mem = get_free_memory() / 1024;
    char mem_str[32];
    char mem_val[10];
    itoa(free_mem, mem_val, 10);
    mem_str[0] = '\0';
    strcat(mem_str, "Free Memory: ");
    strcat(mem_str, mem_val);
    strcat(mem_str, " KB\n");
    terminal_puts(mem_str);
    
    // Uptime
    uint32_t uptime = timer_get_uptime_seconds();
    uint32_t hours = uptime / 3600;
    uint32_t minutes = (uptime % 3600) / 60;
    uint32_t seconds = uptime % 60;
    char uptime_str[32];
    char hour_str[10], min_str[10], sec_str[10];
    itoa(hours, hour_str, 10);
    itoa(minutes, min_str, 10);
    itoa(seconds, sec_str, 10);
    uptime_str[0] = '\0';
    strcat(uptime_str, "Uptime: ");
    if (hours < 10) strcat(uptime_str, "0");
    strcat(uptime_str, hour_str);
    strcat(uptime_str, ":");
    if (minutes < 10) strcat(uptime_str, "0");
    strcat(uptime_str, min_str);
    strcat(uptime_str, ":");
    if (seconds < 10) strcat(uptime_str, "0");
    strcat(uptime_str, sec_str);
    strcat(uptime_str, "\n");
    terminal_puts(uptime_str);
    
    // Time
    rtc_time_t time;
    rtc_get_time(&time);
    char time_str[16];
    char rtc_hour_str[3], rtc_min_str[3], rtc_sec_str[3];
    itoa(time.hour, rtc_hour_str, 10);
    itoa(time.minute, rtc_min_str, 10);
    itoa(time.second, rtc_sec_str, 10);
    time_str[0] = '\0';
    if (time.hour < 10) strcat(time_str, "0");
    strcat(time_str, rtc_hour_str);
    strcat(time_str, ":");
    if (time.minute < 10) strcat(time_str, "0");
    strcat(time_str, rtc_min_str);
    strcat(time_str, ":");
    if (time.second < 10) strcat(time_str, "0");
    strcat(time_str, rtc_sec_str);
    strcat(time_str, "\n");
    terminal_puts("Current Time: ");
    terminal_puts(time_str);
    
    terminal_puts("\n");
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
    shell_register_command("info", "Show system information", cmd_info);
}

void shell_print_prompt(void) {
    terminal_puts("skullos> ");
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
    terminal_puts("\n");
    
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
    terminal_puts("Command not found: ");
    terminal_puts(argv[0]);
    terminal_puts("\nType 'help' for a list of available commands.\n");
}

void shell_run(void) {
    shell_print_prompt();
    
    while (1) {
        // Get a character from the keyboard
        char c = keyboard_getchar();
        
        // Handle special keys
        if (c == '\r' || c == '\n') {
            // Execute command
            terminal_puts("\n");
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
                terminal_puts("\b \b");
            }
        } else if (command_length < MAX_COMMAND_LENGTH - 1 && c >= 32 && c <= 126) {
            // Printable character
            command_buffer[command_length++] = c;
            terminal_putchar(c);
        }
    }
}
