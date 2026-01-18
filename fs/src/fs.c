#include "../include/fs.h"
#include <string.h>
#include "../../kernel/memory.h"  // Memory management functions (kmalloc, kfree)

// The root of the filesystem
fs_node_t *fs_root = 0;

// Default file operations
static uint32_t fs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    // Check if the node has a specific read function
    if (node->read != 0) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}

static uint32_t fs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    // Check if the node has a specific write function
    if (node->write != 0) {
        return node->write(node, offset, size, buffer);
    }
    return 0;
}

static void fs_open(fs_node_t *node, uint8_t read, uint8_t write) {
    // Call the node's open function if it exists
    if (node->open != 0) {
        node->open(node);
    }
}

static void fs_close(fs_node_t *node) {
    // Call the node's close function if it exists
    if (node->close != 0) {
        node->close(node);
    }
}

static struct dirent *fs_readdir(fs_node_t *node, uint32_t index) {
    // Check if the node is a directory and has a readdir function
    if ((node->flags & 0x7) == FS_DIRECTORY && node->readdir != 0) {
        return node->readdir(node, index);
    }
    return 0;
}

static fs_node_t *fs_finddir(fs_node_t *node, char *name) {
    // Check if the node is a directory and has a finddir function
    if ((node->flags & 0x7) == FS_DIRECTORY && node->finddir != 0) {
        return node->finddir(node, name);
    }
    return 0;
}

// Public API functions
uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node) return 0;
    return fs_read(node, offset, size, buffer);
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node) return 0;
    return fs_write(node, offset, size, buffer);
}

void open_fs(fs_node_t *node, uint8_t read, uint8_t write) {
    if (!node) return;
    fs_open(node, read, write);
}

void close_fs(fs_node_t *node) {
    if (!node) return;
    fs_close(node);
}

struct dirent *readdir_fs(fs_node_t *node, uint32_t index) {
    if (!node) return 0;
    return fs_readdir(node, index);
}

fs_node_t *finddir_fs(fs_node_t *node, char *name) {
    if (!node) return 0;
    return fs_finddir(node, name);
}

// Helper function to create a new file node
fs_node_t *make_file(char *name, uint32_t flags, uint32_t size) {
    fs_node_t *node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!node) return 0;
    
    strncpy(node->name, name, 127);
    node->name[127] = '\0';
    node->mask = 0;  // Default permissions
    node->uid = 0;   // Root user
    node->gid = 0;   // Root group
    node->flags = flags | FS_FILE;
    node->inode = 0;  // Will be set by the actual filesystem
    node->length = size;
    node->impl = 0;
    
    // Set function pointers
    node->read = 0;
    node->write = 0;
    node->open = 0;
    node->close = 0;
    node->readdir = 0;
    node->finddir = 0;
    node->ptr = 0;
    node->offset = 0;
    
    return node;
}

// Helper function to create a new directory node
fs_node_t *make_dir(char *name, uint32_t flags) {
    fs_node_t *node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!node) return 0;
    
    strncpy(node->name, name, 127);
    node->name[127] = '\0';
    node->mask = 0;  // Default permissions
    node->uid = 0;   // Root user
    node->gid = 0;   // Root group
    node->flags = flags | FS_DIRECTORY;
    node->inode = 0;  // Will be set by the actual filesystem
    node->length = 0; // Directories don't have a size
    node->impl = 0;
    
    // Set function pointers
    node->read = 0;
    node->write = 0;
    node->open = 0;
    node->close = 0;
    node->readdir = 0;
    node->finddir = 0;
    node->ptr = 0;
    node->offset = 0;
    
    return node;
}

// Initialize the filesystem
void fs_init() {
    // Create the root directory
    fs_root = make_dir("root", 0);
    
    if (!fs_root) {
        // Handle error - couldn't create root directory
        return;
    }
    
    // Set up the root directory's function pointers
    fs_root->readdir = fs_readdir;
    fs_root->finddir = fs_finddir;
    
    // Create some default directories
    fs_node_t *dev = make_dir("dev", 0);
    fs_node_t *proc = make_dir("proc", 0);
    
    // Add them to the root directory
    // Note: You'll need to implement directory linking in your actual filesystem
    
    // For now, we'll just leak these nodes since we don't have a way to free them yet
    (void)dev;
    (void)proc;
}
