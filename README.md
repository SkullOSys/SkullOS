# SkullOS

### Not just another UNIX clone

SkullOS is a hobby operating system written in C and Assembly for the x86 architecture. It is designed to be a minimalistic OS with a custom kernel, a shell, a simple file system, and a few applications.

## Features

*   **32-bit Protected Mode Kernel:** The kernel operates in 32-bit protected mode.
*   **BIOS:** A simple BIOS with a setup utility to configure system settings.
*   **Memory Detection:** The BIOS can detect the available memory.
*   **GUI:** A basic graphical user interface.
*   **Shell:** An interactive shell with a few commands.
*   **Filesystem:** A simple filesystem with an initial ramdisk.
*   **Drivers:** Drivers for keyboard, RTC, and ATA.
*   **Games:** Includes a simple Snake game.
*   **Boot Animation:** A boot animation is shown when the OS starts.

## Project Structure

*   `bios/`: Contains the BIOS code.
*   `boot/`: Contains the 16-bit bootloader.
*   `drivers/`: Contains the device drivers.
*   `fs/`: Contains the file system implementation.
*   `games/`: Contains a snake game.
*   `gui/`: Contains the GUI module.
*   `kernel/`: Contains the kernel source code.
*   `libc/`: Contains a minimalistic C standard library.
*   `tools/`: Contains tools for generating the initial ramdisk.
*   `linker.ld`: The linker script.
*   `Makefile`: The build script.

## Dependencies

To build and run SkullOS, you will need the following tools:

*   `nasm`
*   `qemu`
*   `x86_64-elf-gcc` (Cross-Compiler)

## Building and Running

To build the operating system, run the following command:

```bash
make
```

To clean the build artifacts, run:

```bash
make clean
```

To run the OS in QEMU, use the following command:

```bash
make run
```

This will start QEMU in full-screen mode. You can exit the QEMU window by pressing `Ctrl+Alt+G`. The QEMU monitor can be accessed from the terminal where you launched the `make run` command.