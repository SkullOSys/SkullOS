#include "fs.h"
#include <string.h>
#include "kernel/memory.h"

// Simple initrd file header
struct initrd_file_header {
    char magic[8];      // "INITRD\0\0"
    uint32_t nheaders;   // Number of file headers
} __attribute__((packed));

struct initrd_file_headers {
    char name[64];      // Filename
    uint32_t offset;    // Offset in the initrd
    uint32_t length;    // Length of the file
} __attribute__((packed));

// Global variables
static uint32_t nheaders = 0;
static struct initrd_file_headers *file_headers = 0;
static uint8_t *initrd_start = 0;

// Read a file from the initrd.
static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (offset >= node->length) {
        return 0;  // EOF
    }
    
    if (offset + size > node->length) {
        size = node->length - offset;
    }
    
    memcpy(buffer, initrd_start + node->inode + offset, size);
    return size;
}

static struct dirent dirent;
// Read a directory from the initrd.
static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index) {
    (void)node;
    if (index >= nheaders) {
        return 0;
    }
    strcpy(dirent.name, file_headers[index].name);
    dirent.ino = index;
    return &dirent;
}

// Find a file in the initrd.
static fs_node_t *initrd_finddir(fs_node_t *node, char *name) {
    (void)node;
    for (uint32_t i = 0; i < nheaders; i++) {
        if (strcmp(name, file_headers[i].name) == 0) {
            fs_node_t *file = make_file(file_headers[i].name, 0, file_headers[i].length);
            file->inode = file_headers[i].offset;
            file->read = initrd_read;
            return file;
        }
    }
    return 0;
}


// Initialize the initial ramdisk.
fs_node_t *initrd_initialize(uint32_t location) {
    struct initrd_file_header *header = (struct initrd_file_header*)location;
    
    // Verify the magic number
    if (strncmp(header->magic, "INITRD", 6) != 0) {
        return 0;  // Invalid initrd
    }
    
    nheaders = header->nheaders;
    file_headers = (struct initrd_file_headers*)(location + sizeof(struct initrd_file_header));
    initrd_start = (uint8_t*)(location + sizeof(struct initrd_file_header) + 
                             sizeof(struct initrd_file_headers) * nheaders);
    
    // Create the root directory
    fs_node_t *root = make_dir("initrd", 0);
    root->readdir = initrd_readdir;
    root->finddir = initrd_finddir;
    
    return root;
}
