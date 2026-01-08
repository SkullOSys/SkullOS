# Compiler and flags
ASM = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy

# Flags
ASMFLAGS = -f elf32
CFLAGS = -ffreestanding -m32 -g -fno-pie -Wall -Wextra -I. -std=gnu99
LDFLAGS = -T linker.ld -melf_i386 -nostdlib

# Source files
KERNEL_SRCS = kernel/kernel.c kernel/util.c kernel/vga.c
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o)

GUI_SRCS = gui/gui.c
GUI_OBJS = $(GUI_SRCS:.c=.o)

# Default target
all: os.bin

# Create final OS image
os.bin: boot/boot.bin kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=65
	dd if=boot/boot.bin of=$@ conv=notrunc
	dd if=kernel.bin of=$@ bs=512 seek=1 conv=notrunc

# Bootloader
boot/boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

# Kernel binary
kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

# Kernel ELF file
kernel.elf: $(KERNEL_OBJS) $(GUI_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Kernel objects
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# GUI objects
gui/%.o: gui/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run in QEMU
run: os.bin
	qemu-system-i386 -full-screen -drive format=raw,file=os.bin -monitor stdio

# Debug with QEMU
debug: os.bin
	qemu-system-i386 -drive format=raw,file=os.bin -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

# Clean build artifacts
clean:
	rm -f *.bin *.elf
	rm -f kernel/*.o
	rm -f gui/*.o
	rm -f boot/*.bin

# Phony targets
.PHONY: all clean run debug