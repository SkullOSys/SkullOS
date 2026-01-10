#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

// Command handler type with arguments support
typedef void (*command_handler_t)(int argc, char **argv);

// Initialize the shell
void shell_init(void);

// Run the shell
void shell_run(void);

// Print the shell prompt
void shell_print_prompt(void);

// Register a new command
void shell_register_command(const char* name, const char* description, command_handler_t handler);

// Parse command line into arguments
int shell_parse_arguments(char *line, char **argv, int max_args);

#endif // KERNEL_SHELL_H
