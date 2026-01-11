#include "fs.h"
#include "vga.h"
#include "string.h"
#include "shell.h"

// Global root filesystem node
extern fs_node_t *fs_root;

// Initialize the filesystem
void fs_initialize() {
    // Mount the initial ramdisk as root
    fs_root = initrd_initialize(0x20000);
}

// List directory contents
void list_directory(fs_node_t *node) {
    if ((node->flags & 0x7) != FS_DIRECTORY) {
        vga_put_string("Not a directory\n");
        return;
    }
    
    struct dirent *dir = 0;
    int i = 0;
    while ((dir = readdir_fs(node, i++)) != 0) {
        vga_put_string("  ");
        vga_put_string(dir->name);
        vga_put_string("\n");
    }
}

// Shell command to list directory contents
void cmd_ls(int argc, char **argv) {
    (void)argc; // Unused for now
    (void)argv; // Unused for now
    
    if (!fs_root) {
        vga_put_string("Filesystem not initialized\n");
        return;
    }
    
    vga_put_string("Listing root directory:\n");
    list_directory(fs_root);
}

// Initialize filesystem commands
void fs_init_commands() {
    // Register the 'ls' command
    shell_register_command("ls", "List directory contents", cmd_ls);
}
