#include "fs.h"
#include "terminal.h"
#include "string.h"
#include "shell.h"

// Global root filesystem node
extern fs_node_t *fs_root;

// Forward declaration of initrd_initialize
fs_node_t *initrd_initialize(uint32_t location);

// Directory entry structure for our minimal filesystem
typedef struct {
    char name[128];
    fs_node_t *node;
} minfs_dirent_t;

// Directory structure for our minimal filesystem
typedef struct {
    minfs_dirent_t entries[10];
    uint32_t num_entries;
} minfs_dir_t;

// Minimal filesystem data
static minfs_dir_t root_dir = {0};
static minfs_dir_t dev_dir = {0};
static minfs_dir_t proc_dir = {0};

// Add an entry to a directory
static void minfs_add_entry(minfs_dir_t *dir, const char *name, fs_node_t *node) {
    if (dir->num_entries >= 10) return;
    
    strncpy(dir->entries[dir->num_entries].name, name, 127);
    dir->entries[dir->num_entries].name[127] = '\0';
    dir->entries[dir->num_entries].node = node;
    dir->num_entries++;
}

// Union for type-punning between fs_node* and minfs_dir_t*
typedef union {
    struct fs_node *fs_node;
    minfs_dir_t *dir;
} fs_ptr_union_t;

// Minimal readdir implementation
static struct dirent *minimal_readdir(fs_node_t *node, uint32_t index) {
    fs_ptr_union_t u;
    u.fs_node = node->ptr;
    minfs_dir_t *dir = u.dir;
    static struct dirent dirent;
    
    if (!dir) return 0;
    
    // Special cases for . and ..
    if (index == 0) {
        strcpy(dirent.name, ".");
        dirent.ino = 0;
        return &dirent;
    } else if (index == 1) {
        strcpy(dirent.name, "..");
        dirent.ino = 0;
        return &dirent;
    }
    
    // Regular entries (offset by 2 for . and ..)
    index -= 2;
    if (index < dir->num_entries) {
        strncpy(dirent.name, dir->entries[index].name, 127);
        dirent.name[127] = '\0';
        dirent.ino = index + 2; // Start after . and ..
        return &dirent;
    }
    
    return 0; // No more entries
}

// Minimal finddir implementation
static fs_node_t *minimal_finddir(fs_node_t *node, char *name) {
    fs_ptr_union_t u;
    u.fs_node = node->ptr;
    minfs_dir_t *dir = u.dir;
    
    if (!dir) return 0;
    
    // Check for . and ..
    if (strcmp(name, ".") == 0) {
        return node;
    } else if (strcmp(name, "..") == 0) {
        // For simplicity, just return the node itself as the parent
        return node;
    }
    
    // Check other entries
    for (uint32_t i = 0; i < dir->num_entries; i++) {
        if (strcmp(dir->entries[i].name, name) == 0) {
            return dir->entries[i].node;
        }
    }
    
    return 0; // Not found
}

// Create a minimal filesystem with basic directories
static fs_node_t *create_minimal_fs() {
    // Create root directory
    fs_node_t *root = make_dir("/", 0);
    if (!root) {
        terminal_puts("Failed to create root directory\n");
        return 0;
    }
    
    // Set up root directory
    fs_ptr_union_t u;
    u.dir = &root_dir;
    root->ptr = u.fs_node;
    root->readdir = minimal_readdir;
    root->finddir = minimal_finddir;
    
    // Create default directories
    fs_node_t *dev = make_dir("dev", 0);
    fs_node_t *proc = make_dir("proc", 0);
    
    if (!dev || !proc) {
        terminal_puts("Failed to create default directories\n");
        return 0;
    }
    
    // Set up dev directory
    u.dir = &dev_dir;
    dev->ptr = u.fs_node;
    dev->readdir = minimal_readdir;
    dev->finddir = minimal_finddir;
    
    // Set up proc directory
    u.dir = &proc_dir;
    proc->ptr = u.fs_node;
    proc->readdir = minimal_readdir;
    proc->finddir = minimal_finddir;
    
    // Add entries to the root directory
    minfs_add_entry(&root_dir, "dev", dev);
    minfs_add_entry(&root_dir, "proc", proc);
    
    // Add . and .. to dev and proc
    minfs_add_entry(&dev_dir, ".", dev);
    minfs_add_entry(&dev_dir, "..", root);
    
    minfs_add_entry(&proc_dir, ".", proc);
    minfs_add_entry(&proc_dir, "..", root);
    
    return root;
}

// Initialize the filesystem
void fs_initialize() {
    // Try to mount the initial ramdisk as root
    fs_root = initrd_initialize(0x20000);
    
    // If initrd initialization failed, create a minimal filesystem
    if (!fs_root) {
        terminal_puts("Failed to initialize initrd, falling back to minimal filesystem\n");
        fs_root = create_minimal_fs();
    }
    
    if (!fs_root) {
        terminal_puts("FATAL: Failed to initialize filesystem\n");
        // We can't continue without a filesystem
        while (1) { asm volatile ("hlt"); }
    }
}

// List directory contents
void list_directory(fs_node_t *node) {
    if (!node) {
        terminal_puts("Error: Null directory node\n");
        return;
    }
    
    if ((node->flags & 0x7) != FS_DIRECTORY) {
        terminal_puts("Error: Not a directory\n");
        return;
    }
    
    if (!node->readdir) {
        terminal_puts("Error: Directory cannot be read (no readdir function)\n");
        return;
    }
    
    terminal_puts("Directory listing for ");
    terminal_puts(node->name);
    terminal_puts(":\n");
    
    uint32_t index = 0;
    struct dirent *dir = 0;
    
    while ((dir = node->readdir(node, index++)) != 0) {
        terminal_puts("  ");
        terminal_puts(dir->name);
        
        // Check if this entry is a directory
        if (node->finddir) {
            fs_node_t *entry = node->finddir(node, dir->name);
            if (entry && (entry->flags & 0x7) == FS_DIRECTORY) {
                terminal_puts("/");
            }
        }
        
        terminal_puts("\n");
    }
}

// Shell command to list directory contents
void cmd_ls(int argc, char **argv) {
    (void)argc; // Unused for now
    (void)argv; // Unused for now
    
    if (!fs_root) {
        terminal_puts("Filesystem not initialized\n");
        return;
    }
    
    terminal_puts("Listing root directory:\n");
    list_directory(fs_root);
}

// Initialize filesystem commands
void fs_init_commands() {
    // Register the 'ls' command
    shell_register_command("ls", "List directory contents", cmd_ls);
}
