#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

// Initialize the shell
void shell_init(void);

// Run the shell
void shell_run(void);

// Print the shell prompt
void shell_print_prompt(void);

// Process a command
void shell_process_command(const char* command);

#endif // KERNEL_SHELL_H
