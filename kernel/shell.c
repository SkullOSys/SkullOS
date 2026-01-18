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
#include "fs.h"
#include "../fs/include/fs.h"

// Forward declaration
fs_node_t *resolve_path(const char *path);

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
static void cmd_reboot(int argc, char **argv);
static void cmd_echo(int argc, char **argv);
static void cmd_date(int argc, char **argv);
static void cmd_cat(int argc, char **argv);
static void cmd_version(int argc, char **argv);
static void cmd_pwd(int argc, char **argv);
static void cmd_cd(int argc, char **argv);
static void cmd_sleep(int argc, char **argv);
static void cmd_ps(int argc, char **argv);



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
    size_t used_mem = get_used_memory() / 1024;
    size_t total_mem = get_total_memory() / 1024;
    char mem_str[64];
    char free_val[10], used_val[10], total_val[10];
    itoa(free_mem, free_val, 10);
    itoa(used_mem, used_val, 10);
    itoa(total_mem, total_val, 10);
    mem_str[0] = '\0';
    strcat(mem_str, "Memory: ");
    strcat(mem_str, free_val);
    strcat(mem_str, " KB free, ");
    strcat(mem_str, used_val);
    strcat(mem_str, " KB used, ");
    strcat(mem_str, total_val);
    strcat(mem_str, " KB total\n");
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

static void cmd_reboot(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    terminal_puts("\nRebooting system...\n");
    
    // Wait a bit for the message to be visible
    for (volatile int i = 0; i < 1000000; i++);
    
    // Trigger a reboot via keyboard controller
    uint8_t temp;
    do {
        temp = inb(0x64);
        if (temp & 0x02) {
            // Wait for input buffer to be empty
            continue;
        }
        outb(0x64, 0xFE); // Send reboot command
    } while (1);
}

static void cmd_echo(int argc, char **argv) {
    terminal_puts("\n");
    
    // Print all arguments separated by spaces
    for (int i = 1; i < argc; i++) {
        if (i > 1) {
            terminal_puts(" ");
        }
        terminal_puts(argv[i]);
    }
    
    terminal_puts("\n");
}

static void cmd_date(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    rtc_time_t time;
    rtc_get_time(&time);
    
    char time_str[16];
    char hour_str[3], min_str[3], sec_str[3];
    itoa(time.hour, hour_str, 10);
    itoa(time.minute, min_str, 10);
    itoa(time.second, sec_str, 10);
    
    time_str[0] = '\0';
    if (time.hour < 10) strcat(time_str, "0");
    strcat(time_str, hour_str);
    strcat(time_str, ":");
    if (time.minute < 10) strcat(time_str, "0");
    strcat(time_str, min_str);
    strcat(time_str, ":");
    if (time.second < 10) strcat(time_str, "0");
    strcat(time_str, sec_str);
    
    terminal_puts("\n");
    terminal_puts(time_str);
    terminal_puts("\n");
}

static void cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        terminal_puts("\nUsage: cat <filename>\n");
        return;
    }
    
    extern fs_node_t *fs_root;
    if (!fs_root) {
        terminal_puts("\nFilesystem not initialized\n");
        return;
    }
    
    // Find the file
    fs_node_t *file = finddir_fs(fs_root, argv[1]);
    if (!file) {
        terminal_puts("\nFile not found: ");
        terminal_puts(argv[1]);
        terminal_puts("\n");
        return;
    }
    
    if ((file->flags & 0x7) != FS_FILE) {
        terminal_puts("\nNot a file: ");
        terminal_puts(argv[1]);
        terminal_puts("\n");
        return;
    }
    
    // Read and display the file
    uint8_t buffer[256];
    uint32_t total_read = 0;
    uint32_t file_size = file->length;
    
    terminal_puts("\n");
    
    while (total_read < file_size) {
        uint32_t to_read = file_size - total_read;
        if (to_read > sizeof(buffer)) {
            to_read = sizeof(buffer);
        }
        
        uint32_t read = read_fs(file, total_read, to_read, buffer);
        if (read == 0) break;
        
        // Print the buffer
        for (uint32_t i = 0; i < read; i++) {
            terminal_putchar(buffer[i]);
        }
        
        total_read += read;
    }
    
    terminal_puts("\n");
}

static void cmd_version(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    terminal_puts("\nSkullOS v0.1.0\n");
    terminal_puts("Build date: ");
    terminal_puts(__DATE__);
    terminal_puts(" ");
    terminal_puts(__TIME__);
    terminal_puts("\n\n");
}

// Current working directory (simple implementation)
static char current_dir[256] = "/";

static void cmd_pwd(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    terminal_puts("\n");
    terminal_puts(current_dir);
    terminal_puts("\n");
}

static void cmd_cd(int argc, char **argv) {
    extern fs_node_t *fs_root;
    
    if (argc < 2) {
        // No argument, go to root
        strcpy(current_dir, "/");
        return;
    }
    
    char new_path[256];
    
    // Handle absolute paths
    if (argv[1][0] == '/') {
        strncpy(new_path, argv[1], 255);
        new_path[255] = '\0';
    } else {
        // Relative path
        strncpy(new_path, current_dir, 255);
        if (new_path[strlen(new_path) - 1] != '/') {
            strcat(new_path, "/");
        }
        strncat(new_path, argv[1], 255 - strlen(new_path));
    }
    
    // Resolve path
    fs_node_t *dir = resolve_path(new_path);
    if (dir && (dir->flags & 0x7) == FS_DIRECTORY) {
        strncpy(current_dir, new_path, 255);
        current_dir[255] = '\0';
        // Ensure it ends with /
        if (current_dir[strlen(current_dir) - 1] != '/') {
            strcat(current_dir, "/");
        }
    } else {
        terminal_puts("\nDirectory not found: ");
        terminal_puts(argv[1]);
        terminal_puts("\n");
    }
}

static void cmd_sleep(int argc, char **argv) {
    if (argc < 2) {
        terminal_puts("\nUsage: sleep <seconds>\n");
        return;
    }
    
    // Simple atoi implementation
    int seconds = 0;
    const char *p = argv[1];
    while (*p >= '0' && *p <= '9') {
        seconds = seconds * 10 + (*p - '0');
        p++;
    }
    
    if (seconds > 0) {
        uint32_t start = timer_get_uptime_seconds();
        while (timer_get_uptime_seconds() - start < (uint32_t)seconds) {
            // Busy wait
            asm volatile ("pause");
        }
    }
}

static void cmd_ps(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    terminal_puts("\nPID  Name\n");
    terminal_puts("---  ----\n");
    terminal_puts("  1  shell\n");
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
    shell_register_command("reboot", "Reboot the system", cmd_reboot);
    shell_register_command("echo", "Echo arguments", cmd_echo);
    shell_register_command("date", "Show current time", cmd_date);
    shell_register_command("cat", "Display file contents", cmd_cat);
    shell_register_command("version", "Show OS version", cmd_version);
    shell_register_command("pwd", "Print working directory", cmd_pwd);
    shell_register_command("cd", "Change directory", cmd_cd);
    shell_register_command("sleep", "Sleep for N seconds", cmd_sleep);
    shell_register_command("ps", "List processes", cmd_ps);
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
