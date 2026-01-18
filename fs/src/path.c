#include "../include/fs.h"
#include <string.h>
#include "../../kernel/memory.h"

extern fs_node_t *fs_root;

// Resolve a path string to a filesystem node
fs_node_t *resolve_path(const char *path) {
    if (!path || !fs_root) return 0;
    
    // Handle absolute paths starting with /
    fs_node_t *current = fs_root;
    
    // Skip leading /
    if (*path == '/') {
        path++;
    }
    
    // Empty path means root
    if (*path == '\0') {
        return fs_root;
    }
    
    // Split path by /
    char path_copy[256];
    strncpy(path_copy, path, 255);
    path_copy[255] = '\0';
    
    char *token = path_copy;
    char *saveptr;
    
    // Tokenize path
    char *component = token;
    while (*token) {
        if (*token == '/') {
            *token = '\0';
            if (*component) {
                // Look up component
                if (!current || (current->flags & 0x7) != FS_DIRECTORY) {
                    return 0;
                }
                current = current->finddir(current, component);
                if (!current) {
                    return 0;
                }
            }
            component = token + 1;
        }
        token++;
    }
    
    // Handle last component
    if (*component) {
        if (!current || (current->flags & 0x7) != FS_DIRECTORY) {
            return 0;
        }
        current = current->finddir(current, component);
    }
    
    return current;
}

// Get parent directory from path
fs_node_t *get_parent_dir(const char *path) {
    if (!path || !fs_root) return 0;
    
    // Find last /
    const char *last_slash = 0;
    const char *p = path;
    while (*p) {
        if (*p == '/') {
            last_slash = p;
        }
        p++;
    }
    
    if (!last_slash) {
        // No slash found, parent is current directory (root for now)
        return fs_root;
    }
    
    // Extract parent path
    char parent_path[256];
    int len = last_slash - path;
    if (len > 255) len = 255;
    strncpy(parent_path, path, len);
    parent_path[len] = '\0';
    
    if (len == 0) {
        return fs_root;
    }
    
    return resolve_path(parent_path);
}

// Get filename from path
const char *get_filename(const char *path) {
    if (!path) return 0;
    
    const char *last_slash = 0;
    const char *p = path;
    while (*p) {
        if (*p == '/') {
            last_slash = p;
        }
        p++;
    }
    
    if (!last_slash) {
        return path;
    }
    
    return last_slash + 1;
}

