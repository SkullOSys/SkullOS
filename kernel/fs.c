#include "fs.h"
#include "vga.h"
#include "string.h"
#include "shell.h"

// Global root filesystem node
extern fs_node_t *fs_root;

// Forward declaration of initrd_initialize
fs_node_t *initrd_initialize(uint32_t location);

// Create a minimal filesystem with basic directories
static fs_node_t *create_minimal_fs() {
    // Create root directory
    fs_node_t *root = make_dir("root", 0);
    if (!root) {
        vga_put_string("Failed to create root directory\n");
        return 0;
    }
    
    // Create some default directories
    fs_node_t *dev = make_dir("dev", 0);
    fs_node_t *proc = make_dir("proc", 0);
    
    if (!dev || !proc) {
        vga_put_string("Failed to create default directories\n");
        return 0;
    }
    
    // Note: In a real implementation, you would add these directories to the root
    // For now, we'll just keep them in memory to prevent leaks
    (void)dev;
    (void)proc;
    
    return root;
}

// Initialize the filesystem
void fs_initialize() {
    // Try to mount the initial ramdisk as root
    fs_root = initrd_initialize(0x20000);
    
    // If initrd initialization failed, create a minimal filesystem
    if (!fs_root) {
        vga_put_string("Failed to initialize initrd, falling back to minimal filesystem\n");
        fs_root = create_minimal_fs();
    }
    
    if (!fs_root) {
        vga_put_string("FATAL: Failed to initialize filesystem\n");
        // We can't continue without a filesystem
        while (1) { asm volatile ("hlt"); }
    }
}

// List directory contents
void list_directory(fs_node_t *node) {
    if (!node) {
        vga_put_string("Error: Null directory node\n");
        return;
    }
    
    if ((node->flags & 0x7) != FS_DIRECTORY) {
        vga_put_string("Error: Not a directory\n");
        return;
    }
    
    if (!node->readdir) {
        vga_put_string("Error: Directory cannot be read\n");
        return;
    }
    
    vga_put_string("Directory listing for ");
    vga_put_string(node->name);
    vga_put_string(":\n");
    
    uint32_t index = 0;
    struct dirent *dir = 0;
    
    while ((dir = node->readdir(node, index++)) != 0) {
        vga_put_string("  ");
        vga_put_string(dir->name);
        
        // Get the node to check if it's a directory
        fs_node_t *entry = node->finddir(node, dir->name);
        if (entry && (entry->flags & 0x7) == FS_DIRECTORY) {
            vga_put_string("/");
        }
        
        vga_put_string("\n");
    }
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
