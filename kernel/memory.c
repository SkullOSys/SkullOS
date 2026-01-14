#include "kernel.h"
#include "vga.h"

// Simple memory allocator implementation
// This is a very basic implementation and should be replaced with a proper memory manager

// Memory block header
typedef struct mem_block {
    size_t size;
    struct mem_block *next;
    int free;
} mem_block_t;

// Start of the heap
#define HEAP_START 0x100000  // 1MB mark
#define HEAP_SIZE  0x10000   // 64KB for now

static mem_block_t *free_list = (mem_block_t*)HEAP_START;

// Initialize the memory allocator
void memory_init() {
    free_list->size = HEAP_SIZE - sizeof(mem_block_t);
    free_list->next = NULL;
    free_list->free = 1;
}

// Simple first-fit memory allocator
void* kmalloc(size_t size) {
    // Round up to the nearest 8 bytes for alignment
    size = (size + 7) & ~7;
    
    mem_block_t *current = free_list;
    mem_block_t *prev = NULL;
    (void)prev;
    
    // Find the first free block that's large enough
    while (current) {
        if (current->free && current->size >= size) {
            // Can we split this block?
            if (current->size > size + sizeof(mem_block_t)) {
                // Split the block
                mem_block_t *new_block = (mem_block_t*)((char*)current + sizeof(mem_block_t) + size);
                new_block->size = current->size - size - sizeof(mem_block_t);
                new_block->next = current->next;
                new_block->free = 1;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->free = 0;
            return (void*)(current + 1);
        }
        prev = current;
        current = current->next;
    }
    
    // No suitable block found
    return NULL;
}

// Free allocated memory
void kfree(void *ptr) {
    if (!ptr) return;
    
    mem_block_t *block = (mem_block_t*)ptr - 1;
    block->free = 1;
    
    // Merge with next block if it's free
    if (block->next && block->next->free) {
        block->size += block->next->size + sizeof(mem_block_t);
        block->next = block->next->next;
    }
}

// Get the total free memory in bytes
size_t get_free_memory() {
    size_t free_mem = 0;
    mem_block_t *current = free_list;
    
    while (current) {
        if (current->free) {
            free_mem += current->size;
        }
        current = current->next;
    }
    
    return free_mem;
}

