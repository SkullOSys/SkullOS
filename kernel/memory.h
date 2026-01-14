#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stddef.h>

// Initialize the memory manager
void memory_init(void);

// Memory allocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);

// Get the total free memory in bytes
size_t get_free_memory(void);

#endif // KERNEL_MEMORY_H
