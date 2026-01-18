#include "../include/fs.h"
#include <string.h>
#include "../../kernel/memory.h"

// SkullFS - A simple in-memory filesystem

// File data block
typedef struct file_data {
    uint8_t *data;
    uint32_t size;
    uint32_t capacity;
} file_data_t;

// Directory entry (linked list)
typedef struct dir_entry {
    char name[128];
    fs_node_t *node;
    struct dir_entry *next;
} dir_entry_t;

// Directory structure
typedef struct skullfs_dir {
    dir_entry_t *entries;
    uint32_t num_entries;
    fs_node_t *parent;  // Parent directory
} skullfs_dir_t;

// Global inode counter
static uint32_t next_inode = 1;

// Allocate file data
static file_data_t* allocate_file_data(uint32_t initial_size) {
    file_data_t *fd = (file_data_t*)kmalloc(sizeof(file_data_t));
    if (!fd) return 0;
    
    fd->capacity = initial_size > 0 ? initial_size : 256;
    fd->data = (uint8_t*)kmalloc(fd->capacity);
    if (!fd->data) {
        kfree(fd);
        return 0;
    }
    
    fd->size = 0;
    return fd;
}

// Resize file data if needed
static int resize_file_data(file_data_t *fd, uint32_t new_size) {
    if (new_size <= fd->capacity) {
        return 1;  // No resize needed
    }
    
    // Double capacity until it's enough
    uint32_t new_capacity = fd->capacity;
    while (new_capacity < new_size) {
        new_capacity *= 2;
    }
    
    uint8_t *new_data = (uint8_t*)kmalloc(new_capacity);
    if (!new_data) return 0;
    
    // Copy old data
    memcpy(new_data, fd->data, fd->size);
    
    // Free old data and update
    kfree(fd->data);
    fd->data = new_data;
    fd->capacity = new_capacity;
    
    return 1;
}

// Free file data
static void free_file_data(file_data_t *fd) {
    if (fd) {
        if (fd->data) {
            kfree(fd->data);
        }
        kfree(fd);
    }
}

// SkullFS read function
static uint32_t skullfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node || !buffer) return 0;
    
    file_data_t *fd = (file_data_t*)node->ptr;
    if (!fd || !fd->data) return 0;
    
    if (offset >= fd->size) return 0;  // EOF
    
    if (offset + size > fd->size) {
        size = fd->size - offset;
    }
    
    memcpy(buffer, fd->data + offset, size);
    return size;
}

// SkullFS write function
static uint32_t skullfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node || !buffer) return 0;
    
    file_data_t *fd = (file_data_t*)node->ptr;
    if (!fd) {
        // Allocate file data if it doesn't exist
        fd = allocate_file_data(size + offset);
        if (!fd) return 0;
        node->ptr = (void*)fd;
    }
    
    uint32_t new_size = offset + size;
    if (new_size > fd->size) {
        if (!resize_file_data(fd, new_size)) {
            return 0;
        }
        fd->size = new_size;
        node->length = new_size;
    }
    
    memcpy(fd->data + offset, buffer, size);
    return size;
}

// SkullFS readdir function
static struct dirent *skullfs_readdir(fs_node_t *node, uint32_t index) {
    if (!node) return 0;
    
    skullfs_dir_t *dir = (skullfs_dir_t*)node->ptr;
    if (!dir) return 0;
    
    static struct dirent dirent;
    
    // Special cases for . and ..
    if (index == 0) {
        strcpy(dirent.name, ".");
        dirent.ino = node->inode;
        return &dirent;
    } else if (index == 1) {
        strcpy(dirent.name, "..");
        dirent.ino = dir->parent ? dir->parent->inode : node->inode;
        return &dirent;
    }
    
    // Regular entries
    index -= 2;
    dir_entry_t *entry = dir->entries;
    uint32_t i = 0;
    
    while (entry && i < index) {
        entry = entry->next;
        i++;
    }
    
    if (entry) {
        strncpy(dirent.name, entry->name, 127);
        dirent.name[127] = '\0';
        dirent.ino = entry->node->inode;
        return &dirent;
    }
    
    return 0;
}

// SkullFS finddir function
static fs_node_t *skullfs_finddir(fs_node_t *node, char *name) {
    if (!node || !name) return 0;
    
    skullfs_dir_t *dir = (skullfs_dir_t*)node->ptr;
    if (!dir) return 0;
    
    // Special cases
    if (strcmp(name, ".") == 0) {
        return node;
    } else if (strcmp(name, "..") == 0) {
        return dir->parent ? dir->parent : node;
    }
    
    // Search entries
    dir_entry_t *entry = dir->entries;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->node;
        }
        entry = entry->next;
    }
    
    return 0;
}

// Add entry to directory
static int skullfs_add_entry(skullfs_dir_t *dir, const char *name, fs_node_t *node) {
    if (!dir || !name || !node) return 0;
    
    // Check if entry already exists
    dir_entry_t *entry = dir->entries;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return 0;  // Already exists
        }
        entry = entry->next;
    }
    
    // Create new entry
    dir_entry_t *new_entry = (dir_entry_t*)kmalloc(sizeof(dir_entry_t));
    if (!new_entry) return 0;
    
    strncpy(new_entry->name, name, 127);
    new_entry->name[127] = '\0';
    new_entry->node = node;
    new_entry->next = dir->entries;
    dir->entries = new_entry;
    dir->num_entries++;
    
    return 1;
}

// Remove entry from directory
static int skullfs_remove_entry(skullfs_dir_t *dir, const char *name) {
    if (!dir || !name) return 0;
    
    dir_entry_t *entry = dir->entries;
    dir_entry_t *prev = 0;
    
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                dir->entries = entry->next;
            }
            kfree(entry);
            dir->num_entries--;
            return 1;
        }
        prev = entry;
        entry = entry->next;
    }
    
    return 0;
}

// Create a new file in SkullFS
fs_node_t *skullfs_create_file(fs_node_t *parent_dir, const char *name) {
    if (!parent_dir || !name) return 0;
    
    // Check if parent is a directory
    if ((parent_dir->flags & 0x7) != FS_DIRECTORY) {
        return 0;
    }
    
    skullfs_dir_t *dir = (skullfs_dir_t*)parent_dir->ptr;
    if (!dir) return 0;
    
    // Check if file already exists
    if (skullfs_finddir(parent_dir, name)) {
        return 0;  // Already exists
    }
    
    // Create file node
    fs_node_t *file = make_file(name, 0, 0);
    if (!file) return 0;
    
    file->inode = next_inode++;
    file->read = skullfs_read;
    file->write = skullfs_write;
    file->ptr = 0;  // Will be allocated on first write
    
    // Add to directory
    if (!skullfs_add_entry(dir, name, file)) {
        kfree(file);
        return 0;
    }
    
    return file;
}

// Create a new directory in SkullFS
fs_node_t *skullfs_create_dir(fs_node_t *parent_dir, const char *name) {
    if (!parent_dir || !name) return 0;
    
    // Check if parent is a directory
    if ((parent_dir->flags & 0x7) != FS_DIRECTORY) {
        return 0;
    }
    
    skullfs_dir_t *dir = (skullfs_dir_t*)parent_dir->ptr;
    if (!dir) return 0;
    
    // Check if directory already exists
    if (skullfs_finddir(parent_dir, name)) {
        return 0;  // Already exists
    }
    
    // Create directory node
    fs_node_t *new_dir = make_dir(name, 0);
    if (!new_dir) return 0;
    
    new_dir->inode = next_inode++;
    new_dir->readdir = skullfs_readdir;
    new_dir->finddir = skullfs_finddir;
    
    // Allocate directory structure
    skullfs_dir_t *new_dir_struct = (skullfs_dir_t*)kmalloc(sizeof(skullfs_dir_t));
    if (!new_dir_struct) {
        kfree(new_dir);
        return 0;
    }
    
    new_dir_struct->entries = 0;
    new_dir_struct->num_entries = 0;
    new_dir_struct->parent = parent_dir;
    new_dir->ptr = (void*)new_dir_struct;
    
    // Add to parent directory
    if (!skullfs_add_entry(dir, name, new_dir)) {
        kfree(new_dir_struct);
        kfree(new_dir);
        return 0;
    }
    
    return new_dir;
}

// Delete a file or directory
int skullfs_delete(fs_node_t *parent_dir, const char *name) {
    if (!parent_dir || !name) return 0;
    
    skullfs_dir_t *dir = (skullfs_dir_t*)parent_dir->ptr;
    if (!dir) return 0;
    
    // Find the node
    fs_node_t *node = skullfs_finddir(parent_dir, name);
    if (!node) return 0;
    
    // If it's a directory, make sure it's empty (except . and ..)
    if ((node->flags & 0x7) == FS_DIRECTORY) {
        skullfs_dir_t *node_dir = (skullfs_dir_t*)node->ptr;
        if (node_dir && node_dir->num_entries > 2) {  // More than . and ..
            return 0;  // Directory not empty
        }
        
        // Free directory structure
        if (node_dir) {
            // Free any remaining entries (should only be . and ..)
            dir_entry_t *entry = node_dir->entries;
            while (entry) {
                dir_entry_t *next = entry->next;
                kfree(entry);
                entry = next;
            }
            kfree(node_dir);
        }
    } else {
        // Free file data
        file_data_t *fd = (file_data_t*)node->ptr;
        if (fd) {
            free_file_data(fd);
        }
    }
    
    // Remove from parent directory
    skullfs_remove_entry(dir, name);
    
    // Free the node
    kfree(node);
    
    return 1;
}

// Initialize SkullFS root
fs_node_t *skullfs_init() {
    // Create root directory
    fs_node_t *root = make_dir("/", 0);
    if (!root) return 0;
    
    root->inode = next_inode++;
    root->readdir = skullfs_readdir;
    root->finddir = skullfs_finddir;
    
    // Allocate root directory structure
    skullfs_dir_t *root_dir = (skullfs_dir_t*)kmalloc(sizeof(skullfs_dir_t));
    if (!root_dir) {
        kfree(root);
        return 0;
    }
    
    root_dir->entries = 0;
    root_dir->num_entries = 0;
    root_dir->parent = 0;  // Root has no parent
    root->ptr = (void*)root_dir;
    
    // Create default directories
    skullfs_create_dir(root, "dev");
    skullfs_create_dir(root, "proc");
    skullfs_create_dir(root, "tmp");
    
    return root;
}

