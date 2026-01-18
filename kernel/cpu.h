#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char vendor[13];
    bool has_cpuid;
    bool has_sse;
    bool has_sse2;
    bool has_mmx;
} cpu_info_t;

void cpu_init(void);
cpu_info_t* cpu_get_info(void);

#endif // KERNEL_CPU_H

