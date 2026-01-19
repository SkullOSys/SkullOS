#include "syscall.h"
#include "kernel.h"
#include "vga_manager.h"
#include "terminal.h"
#include "memory.h"
#include "timer.h"
#include "fs.h"
#include "../fs/include/fs.h"
#include "idt.h"
#include <string.h>

// External assembly function
extern void syscall_handler_asm(void);

// Current process ID (simple implementation)
static int current_pid = 1;

// System call handler (called from assembly)
// Arguments are passed via registers: eax=syscall_num, ebx=arg1, ecx=arg2, edx=arg3
void syscall_handler(void) {
    uint32_t syscall_num, arg1, arg2, arg3;
    
    // Read from registers (they're saved on stack by pushad)
    // After pushad, registers are at: [esp+28]=eax, [esp+24]=ecx, [esp+20]=edx, [esp+16]=ebx
    asm volatile (
        "movl 28(%%esp), %0\n\t"  // eax = syscall_num
        "movl 16(%%esp), %1\n\t"  // ebx = arg1
        "movl 24(%%esp), %2\n\t"  // ecx = arg2
        "movl 20(%%esp), %3"      // edx = arg3
        : "=r" (syscall_num), "=r" (arg1), "=r" (arg2), "=r" (arg3)
        :
        : "memory"
    );
    
    // Dispatch the system call
    uint32_t result = syscall_dispatcher(syscall_num, arg1, arg2, arg3);
    
    // Store result back in saved eax position (will be restored by popad)
    asm volatile (
        "movl %0, 28(%%esp)"
        :
        : "r" (result)
        : "memory"
    );
}

// Initialize system calls
void syscall_init(void) {
    // Set up interrupt 0x80 for system calls
    idt_set_gate(0x80, (uint32_t)syscall_handler_asm, KERNEL_CS, IDT_FLAG_32BIT_INTERRUPT | IDT_FLAG_RING3 | IDT_FLAG_PRESENT);
}

// System call dispatcher
uint32_t syscall_dispatcher(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    switch (syscall_num) {
        case SYS_EXIT:
            sys_exit((int)arg1);
            return 0;
            
        case SYS_WRITE:
            return sys_write((int)arg1, (const char*)arg2, arg3);
            
        case SYS_READ:
            return sys_read((int)arg1, (char*)arg2, arg3);
            
        case SYS_OPEN:
            return sys_open((const char*)arg1, (int)arg2);
            
        case SYS_CLOSE:
            return sys_close((int)arg1);
            
        case SYS_GETPID:
            return sys_getpid();
            
        case SYS_SLEEP:
            return sys_sleep(arg1);
            
        case SYS_MALLOC:
            return (uint32_t)sys_malloc(arg1);
            
        case SYS_FREE:
            sys_free((void*)arg1);
            return 0;
            
        default:
            return (uint32_t)-1;  // Invalid system call
    }
}

// System call implementations
void sys_exit(int status) {
    (void)status;  // For now, just ignore status
    // In a real OS, this would clean up the process
    // For now, we'll just halt
    terminal_puts("\nProcess exited\n");
}

uint32_t sys_write(int fd, const char *buf, uint32_t count) {
    if (fd == 1 || fd == 2) {  // stdout or stderr
        for (uint32_t i = 0; i < count; i++) {
            terminal_putchar(buf[i]);
        }
        return count;
    }
    return 0;
}

uint32_t sys_read(int fd, char *buf, uint32_t count) {
    (void)fd;  // For now, only stdin (fd 0)
    (void)buf;
    (void)count;
    // TODO: Implement keyboard input reading
    return 0;
}

int sys_open(const char *pathname, int flags) {
    (void)flags;  // Ignore flags for now
    
    fs_node_t *file = resolve_path(pathname);
    
    if (file && (file->flags & 0x7) == FS_FILE) {
        return (int)(uintptr_t)file;  // Use pointer as file descriptor
    }
    
    return -1;  // File not found
}

int sys_close(int fd) {
    (void)fd;  // For now, nothing to clean up
    return 0;
}

int sys_getpid(void) {
    return current_pid;
}

int sys_sleep(uint32_t seconds) {
    uint32_t start_time = timer_get_uptime_seconds();
    while (timer_get_uptime_seconds() - start_time < seconds) {
        // Busy wait - in a real OS, this would yield to other processes
        asm volatile ("pause");
    }
    return 0;
}

void* sys_malloc(uint32_t size) {
    return kmalloc(size);
}

void sys_free(void *ptr) {
    kfree(ptr);
}

