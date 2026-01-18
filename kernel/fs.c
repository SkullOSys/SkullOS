#include "fs.h"
#include "terminal.h"
#include "string.h"
#include "shell.h"
#include "util.h"

// Global root filesystem node
extern fs_node_t *fs_root;

// Forward declarations
fs_node_t *initrd_initialize(uint32_t location);
fs_node_t *skullfs_init(void);
fs_node_t *skullfs_create_file(fs_node_t *parent_dir, const char *name);
fs_node_t *skullfs_create_dir(fs_node_t *parent_dir, const char *name);
int skullfs_delete(fs_node_t *parent_dir, const char *name);
fs_node_t *resolve_path(const char *path);
fs_node_t *get_parent_dir(const char *path);
const char *get_filename(const char *path);

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
    
    // If initrd initialization failed, use SkullFS
    if (!fs_root) {
        terminal_puts("Initializing SkullFS...\n");
        fs_root = skullfs_init();
    }
    
    // If SkullFS initialization failed, fall back to minimal filesystem
    if (!fs_root) {
        terminal_puts("Failed to initialize SkullFS, falling back to minimal filesystem\n");
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
    fs_node_t *dir = fs_root;
    
    if (argc > 1) {
        dir = resolve_path(argv[1]);
        if (!dir) {
            terminal_puts("\nDirectory not found: ");
            terminal_puts(argv[1]);
            terminal_puts("\n");
            return;
        }
    }
    
    if (!fs_root) {
        terminal_puts("Filesystem not initialized\n");
        return;
    }
    
    if ((dir->flags & 0x7) != FS_DIRECTORY) {
        terminal_puts("\nNot a directory: ");
        if (argc > 1) terminal_puts(argv[1]);
        terminal_puts("\n");
        return;
    }
    
    list_directory(dir);
}

// Shell command to create a directory
void cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        terminal_puts("\nUsage: mkdir <directory>\n");
        return;
    }
    
    if (!fs_root) {
        terminal_puts("\nFilesystem not initialized\n");
        return;
    }
    
    fs_node_t *parent = get_parent_dir(argv[1]);
    const char *name = get_filename(argv[1]);
    
    if (!parent) {
        terminal_puts("\nParent directory not found\n");
        return;
    }
    
    if ((parent->flags & 0x7) != FS_DIRECTORY) {
        terminal_puts("\nParent is not a directory\n");
        return;
    }
    
    fs_node_t *new_dir = skullfs_create_dir(parent, name);
    if (new_dir) {
        terminal_puts("\nDirectory created: ");
        terminal_puts(argv[1]);
        terminal_puts("\n");
    } else {
        terminal_puts("\nFailed to create directory (may already exist)\n");
    }
}

// Shell command to create a file
void cmd_touch(int argc, char **argv) {
    if (argc < 2) {
        terminal_puts("\nUsage: touch <filename>\n");
        return;
    }
    
    if (!fs_root) {
        terminal_puts("\nFilesystem not initialized\n");
        return;
    }
    
    fs_node_t *parent = get_parent_dir(argv[1]);
    const char *name = get_filename(argv[1]);
    
    if (!parent) {
        terminal_puts("\nParent directory not found\n");
        return;
    }
    
    if ((parent->flags & 0x7) != FS_DIRECTORY) {
        terminal_puts("\nParent is not a directory\n");
        return;
    }
    
    fs_node_t *file = skullfs_create_file(parent, name);
    if (file) {
        terminal_puts("\nFile created: ");
        terminal_puts(argv[1]);
        terminal_puts("\n");
    } else {
        terminal_puts("\nFailed to create file (may already exist)\n");
    }
}

// Shell command to delete a file or directory
void cmd_rm(int argc, char **argv) {
    if (argc < 2) {
        terminal_puts("\nUsage: rm <file_or_directory>\n");
        return;
    }
    
    if (!fs_root) {
        terminal_puts("\nFilesystem not initialized\n");
        return;
    }
    
    fs_node_t *parent = get_parent_dir(argv[1]);
    const char *name = get_filename(argv[1]);
    
    if (!parent) {
        terminal_puts("\nParent directory not found\n");
        return;
    }
    
    if ((parent->flags & 0x7) != FS_DIRECTORY) {
        terminal_puts("\nParent is not a directory\n");
        return;
    }
    
    if (skullfs_delete(parent, name)) {
        terminal_puts("\nDeleted: ");
        terminal_puts(argv[1]);
        terminal_puts("\n");
    } else {
        terminal_puts("\nFailed to delete (file not found or directory not empty)\n");
    }
}

// Shell command to write to a file
void cmd_write(int argc, char **argv) {
    if (argc < 3) {
        terminal_puts("\nUsage: write <filename> <text>\n");
        return;
    }
    
    if (!fs_root) {
        terminal_puts("\nFilesystem not initialized\n");
        return;
    }
    
    fs_node_t *file = resolve_path(argv[1]);
    if (!file) {
        // File doesn't exist, try to create it
        fs_node_t *parent = get_parent_dir(argv[1]);
        const char *name = get_filename(argv[1]);
        
        if (!parent) {
            terminal_puts("\nParent directory not found\n");
            return;
        }
        
        file = skullfs_create_file(parent, name);
        if (!file) {
            terminal_puts("\nFailed to create file\n");
            return;
        }
    }
    
    if ((file->flags & 0x7) != FS_FILE) {
        terminal_puts("\nNot a file\n");
        return;
    }
    
    // Concatenate all arguments after filename
    char text[256] = {0};
    for (int i = 2; i < argc; i++) {
        if (i > 2) {
            strcat(text, " ");
        }
        strncat(text, argv[i], 255 - strlen(text));
    }
    
    uint32_t len = strlen(text);
    if (write_fs(file, 0, len, (uint8_t*)text) == len) {
        terminal_puts("\nWritten ");
        char len_str[10];
        itoa(len, len_str, 10);
        terminal_puts(len_str);
        terminal_puts(" bytes to ");
        terminal_puts(argv[1]);
        terminal_puts("\n");
    } else {
        terminal_puts("\nFailed to write to file\n");
    }
}

// Initialize filesystem commands
void fs_init_commands() {
    // Register filesystem commands
    shell_register_command("ls", "List directory contents", cmd_ls);
    shell_register_command("mkdir", "Create a directory", cmd_mkdir);
    shell_register_command("touch", "Create an empty file", cmd_touch);
    shell_register_command("rm", "Delete a file or directory", cmd_rm);
    shell_register_command("write", "Write text to a file", cmd_write);
}
