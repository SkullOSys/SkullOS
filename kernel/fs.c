#include "fs.h"
#include "vga.h"
#include "string.h"
#include "shell.h"

// Global root filesystem node
extern fs_node_t *fs_root;

// Forward declaration of initrd_initialize
fs_node_t *initrd_initialize(uint32_t location);

// Minimal directory entry array for the root directory
#define MAX_ENTRIES 10
static struct dirent dir_entries[MAX_ENTRIES];
static uint32_t num_entries = 0;

// Minimal readdir implementation for the root directory
static struct dirent *minimal_readdir(fs_node_t *node, uint32_t index) {
    (void)node; // Unused parameter
    
    // Special case: . and .. entries
    if (index == 0) {
        strcpy(dir_entries[0].name, ".");
        dir_entries[0].ino = 0;
        return &dir_entries[0];
    } else if (index == 1) {
        strcpy(dir_entries[1].name, "..");
        dir_entries[1].ino = 0;
        return &dir_entries[1];
    } else if (index - 2 < num_entries) {
        return &dir_entries[index - 2];
    }
    
    return 0; // No more entries
}

// Minimal finddir implementation
static fs_node_t *minimal_finddir(fs_node_t *node, char *name) {
    // Check for . and ..
    if (strcmp(name, ".") == 0) {
        return node;
    }
    
    // In the minimal implementation, we'll just check if the name exists in our entries
    for (uint32_t i = 0; i < num_entries; i++) {
        if (strcmp(dir_entries[i].name, name) == 0) {
            // In a real implementation, we would return the actual node
            // For now, we'll just return the node itself as a placeholder
            return node;
        }
    }
    
    return 0; // Not found
}

// Add an entry to the root directory
static void add_dir_entry(const char *name) {
    if (num_entries < MAX_ENTRIES) {
        strncpy(dir_entries[num_entries].name, name, 127);
        dir_entries[num_entries].name[127] = '\0';
        dir_entries[num_entries].ino = num_entries + 2; // Start after . and ..
        num_entries++;
    }
}

// Create a minimal filesystem with basic directories
static fs_node_t *create_minimal_fs() {
    // Reset entries
    num_entries = 0;
    
    // Create root directory
    fs_node_t *root = make_dir("root", 0);
    if (!root) {
        vga_put_string("Failed to create root directory\n");
        return 0;
    }
    
    // Set up root directory functions
    root->readdir = minimal_readdir;
    root->finddir = minimal_finddir;
    
    // Create and add some default directories
    fs_node_t *dev = make_dir("dev", 0);
    fs_node_t *proc = make_dir("proc", 0);
    
    if (!dev || !proc) {
        vga_put_string("Failed to create default directories\n");
        return 0;
    }
    
    // Set up directory functions for subdirectories
    dev->readdir = minimal_readdir;
    dev->finddir = minimal_finddir;
    
    proc->readdir = minimal_readdir;
    proc->finddir = minimal_finddir;
    
    // Add entries to the root directory
    add_dir_entry("dev");
    add_dir_entry("proc");
    
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
        vga_put_string("Error: Directory cannot be read (no readdir function)\n");
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
        
        // Check if this entry is a directory
        if (node->finddir) {
            fs_node_t *entry = node->finddir(node, dir->name);
            if (entry && (entry->flags & 0x7) == FS_DIRECTORY) {
                vga_put_string("/");
            }
        }
        
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
