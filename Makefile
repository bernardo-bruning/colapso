# Makefile para o Colapso OS V4.2 (Multiple Apps)

AS = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fno-pic -fno-stack-protector
LDFLAGS = -m elf_i386 -T scripts/kernel.ld

IMG = build/colapso.img
KERNEL_BIN = build/kernel.bin
BOOT_BIN = build/boot.bin
BASH_BIN = build/bash.bin
HELLO_BIN = build/hello.bin

KERNEL_OBJS = build/kernel_entry.o build/interrupts.o build/kernel.o build/idt.o build/keyboard.o build/disk.o build/syscall.o
BASH_OBJS = build/bash_entry.o build/bash.o

all: $(IMG)

$(IMG): $(BOOT_BIN) $(KERNEL_BIN) $(BASH_BIN) $(HELLO_BIN) scripts/mkfs.py
	mkdir -p build
	python3 scripts/mkfs.py
	cat $(BOOT_BIN) $(KERNEL_BIN) > build/temp.bin
	dd if=/dev/zero of=$(IMG) bs=1M count=10
	dd if=build/temp.bin of=$(IMG) conv=notrunc
	# Diretório
	dd if=build/directory.bin of=$(IMG) bs=512 seek=100 conv=notrunc
	# Apps
	dd if=$(BASH_BIN) of=$(IMG) bs=512 seek=40 conv=notrunc
	dd if=$(HELLO_BIN) of=$(IMG) bs=512 seek=60 conv=notrunc
	rm build/temp.bin

$(BOOT_BIN): src/boot/boot.asm src/boot/gdt.asm
	$(AS) -f bin src/boot/boot.asm -o $@

$(HELLO_BIN): src/apps/hello.asm
	$(AS) -f bin src/apps/hello.asm -o $@

$(BASH_BIN): $(BASH_OBJS) scripts/bash.ld
	$(LD) -m elf_i386 -T scripts/bash.ld $(BASH_OBJS) -o $@

build/bash_entry.o: src/apps/bash_entry.asm
	$(AS) -f elf32 src/apps/bash_entry.asm -o $@

build/bash.o: src/apps/bash.c
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel_entry.o: src/kernel/kernel_entry.asm
	$(AS) -f elf32 src/kernel/kernel_entry.asm -o $@

build/interrupts.o: src/kernel/interrupts.asm
	$(AS) -f elf32 src/kernel/interrupts.asm -o $@

build/idt.o: src/kernel/idt.c
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: src/kernel/keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@

build/disk.o: src/kernel/disk.c
	$(CC) $(CFLAGS) -c $< -o $@

build/syscall.o: src/kernel/syscall.c
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.o: src/kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): $(KERNEL_OBJS) scripts/kernel.ld
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o $@

clean: stop
	rm -rf build/*.o build/*.bin build/*.img

stop:
	-pkill -f qemu-system-i386

run-vnc: all
	@echo "Iniciando QEMU..."
	qemu-system-i386 -hda $(IMG) -vnc :0 -display none &
