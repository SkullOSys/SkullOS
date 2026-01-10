#ifndef KERNEL_FS_H
#define KERNEL_FS_H

#include "../fs/include/fs.h"

// Initialize the filesystem
void fs_initialize();

// List directory contents
void list_directory(fs_node_t *node);

// Shell command to list directory
void cmd_ls(int argc, char **argv);

// Initialize filesystem commands
void fs_init_commands();

#endif // KERNEL_FS_H
