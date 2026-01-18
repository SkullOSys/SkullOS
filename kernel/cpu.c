#include "cpu.h"
#include "kernel.h"
#include <string.h>

static cpu_info_t cpu_info;

// Check if CPUID is supported
static bool cpu_has_cpuid(void) {
    uint32_t eflags;
    asm volatile (
        "pushfl\n\t"
        "popl %0\n\t"
        : "=r" (eflags)
    );
    
    // Try to toggle ID bit
    uint32_t eflags_id = eflags ^ (1 << 21);
    asm volatile (
        "pushl %0\n\t"
        "popfl\n\t"
        "pushfl\n\t"
        "popl %0\n\t"
        : "=r" (eflags)
        : "0" (eflags_id)
    );
    
    return (eflags & (1 << 21)) != 0;
}

// Execute CPUID instruction
static void cpuid(uint32_t eax, uint32_t* eax_out, uint32_t* ebx_out, uint32_t* ecx_out, uint32_t* edx_out) {
    asm volatile (
        "cpuid"
        : "=a" (*eax_out), "=b" (*ebx_out), "=c" (*ecx_out), "=d" (*edx_out)
        : "a" (eax)
    );
}

// Initialize CPU detection
void cpu_init(void) {
    memset(&cpu_info, 0, sizeof(cpu_info));
    
    cpu_info.has_cpuid = cpu_has_cpuid();
    
    if (cpu_info.has_cpuid) {
        uint32_t eax, ebx, ecx, edx;
        
        // Get vendor string
        cpuid(0, &eax, &ebx, &ecx, &edx);
        
        // Extract vendor string (12 bytes)
        uint32_t vendor[4] = {ebx, edx, ecx, 0};
        memcpy(cpu_info.vendor, vendor, 12);
        cpu_info.vendor[12] = '\0';
        
        // Get feature flags
        cpuid(1, &eax, &ebx, &ecx, &edx);
        
        cpu_info.has_mmx = (edx & (1 << 23)) != 0;
        cpu_info.has_sse = (edx & (1 << 25)) != 0;
        cpu_info.has_sse2 = (edx & (1 << 26)) != 0;
    } else {
        // Fallback if CPUID not available
        strcpy(cpu_info.vendor, "Unknown");
    }
}

cpu_info_t* cpu_get_info(void) {
    return &cpu_info;
}

