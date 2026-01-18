#ifndef _FS_H
#define _FS_H

#include <stdint.h>
#include <stddef.h>

// File system types
typedef enum {
    FS_FILE = 1,
    FS_DIRECTORY,
    FS_CHAR_DEVICE,
    FS_BLOCK_DEVICE,
    FS_PIPE,
    FS_SYMLINK,
    FS_MOUNTPOINT
} fs_node_type_t;

typedef struct fs_node fs_node_t;

typedef uint32_t (*read_type_t)(fs_node_t*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(fs_node_t*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(fs_node_t*);
typedef void (*close_type_t)(fs_node_t*);
typedef struct dirent* (*readdir_type_t)(fs_node_t*, uint32_t);
typedef fs_node_t* (*finddir_type_t)(fs_node_t*, char *name);

// File system node structure
typedef struct fs_node {
    char name[128];             // Filename
    uint32_t mask;              // Permission mask
    uint32_t uid;               // Owner user
    uint32_t gid;               // Owner group
    uint32_t flags;             // Includes node type
    uint32_t inode;             // Inode number
    uint32_t length;            // File size in bytes
    uint32_t impl;              // Implementation-defined number
    
    // Function pointers
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    
    struct fs_node *ptr;        // Used by mountpoints and symlinks
    uint32_t offset;            // Offset for file position
} fs_node_t;

// Directory entry structure
struct dirent {
    char name[128];
    uint32_t ino;               // Inode number
};

// Standard file system functions
extern fs_node_t *fs_root;      // The root of the filesystem

// File system function declarations
uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fs(fs_node_t *node, uint8_t read, uint8_t write);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

// Helper functions
fs_node_t *make_file(char *name, uint32_t flags, uint32_t size);
fs_node_t *make_dir(char *name, uint32_t flags);

// Initrd initialization
fs_node_t *initrd_initialize(uint32_t location);

// SkullFS functions
fs_node_t *skullfs_init(void);
fs_node_t *skullfs_create_file(fs_node_t *parent_dir, const char *name);
fs_node_t *skullfs_create_dir(fs_node_t *parent_dir, const char *name);
int skullfs_delete(fs_node_t *parent_dir, const char *name);

// Path resolution
fs_node_t *resolve_path(const char *path);
fs_node_t *get_parent_dir(const char *path);
const char *get_filename(const char *path);

// Initialization function
void fs_initialize_vfs();

#endif // _FS_H
