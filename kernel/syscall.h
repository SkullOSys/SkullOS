#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <stdint.h>

// System call numbers
#define SYS_EXIT        1
#define SYS_WRITE       2
#define SYS_READ        3
#define SYS_OPEN        4
#define SYS_CLOSE       5
#define SYS_FORK        6
#define SYS_EXEC        7
#define SYS_GETPID      8
#define SYS_SLEEP       9
#define SYS_MALLOC      10
#define SYS_FREE        11

// System call handler
void syscall_handler(void);

// Initialize system calls
void syscall_init(void);

// System call dispatcher
uint32_t syscall_dispatcher(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// Individual system call implementations
void sys_exit(int status);
uint32_t sys_write(int fd, const char *buf, uint32_t count);
uint32_t sys_read(int fd, char *buf, uint32_t count);
int sys_open(const char *pathname, int flags);
int sys_close(int fd);
int sys_fork(void);
int sys_exec(const char *path, char *const argv[]);
int sys_getpid(void);
int sys_sleep(uint32_t seconds);
void* sys_malloc(uint32_t size);
void sys_free(void *ptr);

#endif // KERNEL_SYSCALL_H

