# Makefile para o Colapso OS V5.0 (Unix-like Structure)

AS = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fno-pic -fno-stack-protector -mno-mmx -mno-sse -mno-sse2
LDFLAGS = -m elf_i386 -T scripts/kernel.ld

IMG = build/colapso.img
KERNEL_BIN = build/kernel.bin
BOOT_BIN = build/boot.bin
BASH_BIN = build/bash.bin
LS_BIN = build/ls.bin
CAT_BIN = build/cat.bin
HELLO_BIN = build/hello.bin

KERNEL_OBJS = build/kernel_entry.o build/interrupts.o build/kernel.o build/idt.o build/keyboard.o build/disk.o build/syscall.o
BASH_OBJS = build/bash_entry.o build/bash.o
APP_ENTRY = build/app_entry.o

all: $(IMG)

$(IMG): $(BOOT_BIN) $(KERNEL_BIN) $(BASH_BIN) $(LS_BIN) $(CAT_BIN) $(HELLO_BIN) README.txt scripts/mkfs.py
	mkdir -p build
	python3 scripts/mkfs.py
	cat $(BOOT_BIN) $(KERNEL_BIN) > build/temp.bin
	dd if=/dev/zero of=$(IMG) bs=1M count=10
	dd if=build/temp.bin of=$(IMG) conv=notrunc
	dd if=build/directory.bin of=$(IMG) bs=512 seek=100 conv=notrunc
	dd if=$(BASH_BIN) of=$(IMG) bs=512 seek=200 conv=notrunc
	dd if=$(LS_BIN) of=$(IMG) bs=512 seek=60 conv=notrunc
	dd if=$(CAT_BIN) of=$(IMG) bs=512 seek=80 conv=notrunc
	dd if=$(HELLO_BIN) of=$(IMG) bs=512 seek=70 conv=notrunc
	dd if=README.txt of=$(IMG) bs=512 seek=90 conv=notrunc
	rm build/temp.bin

$(BOOT_BIN): src/boot/boot.asm src/boot/gdt.asm
	$(AS) -f bin src/boot/boot.asm -o $@

# --- REGRAS DE APPS ---
$(APP_ENTRY): src/apps/app_entry.asm
	$(AS) -f elf32 src/apps/app_entry.asm -o $@

build/ls.o: src/apps/ls.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LS_BIN): $(APP_ENTRY) build/ls.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/ls.o -o $@

build/cat.o: src/apps/cat.c
	$(CC) $(CFLAGS) -c $< -o $@

$(CAT_BIN): $(APP_ENTRY) build/cat.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/cat.o -o $@

$(HELLO_BIN): src/apps/hello.asm
	$(AS) -f bin src/apps/hello.asm -o $@

# Bash (O único que usa o bash_entry com reset de stack)
build/bash_entry.o: src/apps/bash_entry.asm
	$(AS) -f elf32 src/apps/bash_entry.asm -o $@

build/bash.o: src/apps/bash.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BASH_BIN): build/bash_entry.o build/bash.o scripts/bash.ld
	$(LD) -m elf_i386 -T scripts/bash.ld build/bash_entry.o build/bash.o -o $@

# Kernel
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

run-vnc: stop all
	@echo "Iniciando QEMU..."
	qemu-system-i386 -drive file=$(IMG),format=raw -vnc :0 -display none -serial file:build/serial.log -no-reboot &

test: all
	python3 tests/validate_chain.py
	python3 tests/validate_ls.py
	python3 tests/validate_cat.py
