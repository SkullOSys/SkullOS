ENTRY(kernel_entry)

SECTIONS {
    /* The kernel is loaded at 0x8000 in physical memory */
    . = 0x8000;

    /* Kernel entry point must be at the beginning */
    .text : {
        *(.text.entry)
        *(.text .text.*)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata .rodata.*)
    }

    /* Read-write data (initialized) */
    .data : {
        *(.data .data.*)
    }

    /* Read-write data (uninitialized) and stack */
    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    }

    /* End of kernel */
    _kernel_end = .;
}
