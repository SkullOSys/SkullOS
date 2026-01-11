# Compiler and flags
ASM = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy

# Flags
ASMFLAGS = -f elf32
CFLAGS = -ffreestanding -m32 -g -fno-pie -Wall -Wextra -I./libc/include -I. -I./fs/include -I./kernel -std=gnu99 -nostdlib -nostdinc -fno-builtin
LDFLAGS = -T linker.ld -melf_i386 -nostdlib

# Host tools
HOSTCC = gcc
HOSTCFLAGS = -O2

# Source files
LIBC_SRCS = libc/string.c
KERNEL_SRCS = kernel/kernel.c kernel/util.c kernel/vga.c kernel/shell.c kernel/idt.c kernel/pic.c kernel/fs.c kernel/memory.c $(LIBC_SRCS)
FS_SRCS = fs/src/fs.c fs/src/initrd.c
FS_OBJS = $(FS_SRCS:.c=.o)
ASM_SRCS = kernel/interrupts.asm
DRIVER_SRCS = drivers/keyboard/keyboard.c
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o) $(DRIVER_SRCS:.c=.o) $(ASM_SRCS:.asm=.o)

GUI_SRCS = gui/gui.c
GUI_OBJS = $(GUI_SRCS:.c=.o)

# Default target
all: os.bin initrd.bin

# Create final OS image
os.bin: boot/boot.bin kernel.bin initrd.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=boot/boot.bin of=$@ conv=notrunc
	dd if=kernel.bin of=$@ bs=512 seek=1 conv=notrunc
	dd if=initrd.bin of=$@ bs=512 seek=65 conv=notrunc

# Bootloader
boot/boot.bin: boot/boot.asm
	$(ASM) -f bin $< -o $@

# Kernel binary
kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

# Kernel ELF file
kernel.elf: $(KERNEL_OBJS) $(GUI_OBJS) $(FS_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Kernel objects
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling assembly files
%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Rule for compiling driver files
drivers/%.o: drivers/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling libc files
libc/%.o: libc/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling GUI files
gui/%.o: gui/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for compiling fs files
fs/src/%.o: fs/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Host tools build
tools/geninitrd: tools/geninitrd.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

initrd.bin: tools/geninitrd hello.txt
	./tools/geninitrd $@ hello.txt

# Clean build artifacts
clean:
	rm -f *.bin *.elf
	rm -f kernel/*.o
	rm -f gui/*.o
	rm -f boot/*.bin
	rm -f drivers/*.o
	rm -f libc/*.o

# Run the OS in QEMU
run: os.bin
	qemu-system-i386 -full-screen -drive format=raw,file=os.bin -monitor stdio

.PHONY: all clean run