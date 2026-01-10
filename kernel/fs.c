#include "fs.h"
#include "vga.h"
#include "string.h"
#include "shell.h"

// Forward declarations for functions used before their definition
void cmd_ls(int argc, char **argv);

// Global root filesystem node
extern fs_node_t *fs_root;

// Initialize the filesystem
void fs_initialize() {
    // Initialize the VFS
    fs_initialize_vfs();
    
    // Here you would initialize your actual filesystem
    // For now, we'll create a simple root directory with a test file
    // fs_root = make_dir("root", 0);
    
    // Add some test files
    fs_node_t *test_file = make_file("test.txt", 0, 15);
    // In a real implementation, you would add this to a directory
    (void)test_file;  // Prevent unused variable warning
    
    // You would also initialize your actual filesystem here
    // For example: fs_root = initrd_initialize(initrd_location);
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

// Shell command to list directory
// Shell command to list directory
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
