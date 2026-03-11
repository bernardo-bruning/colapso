# Makefile para o Colapso OS V2.2 (Correção de Colisão de Disco)

AS = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fno-pic -fno-stack-protector
LDFLAGS = -m elf_i386 -T scripts/kernel.ld

IMG = build/colapso.img
KERNEL_BIN = build/kernel.bin
BOOT_BIN = build/boot.bin
APP_BIN = build/app.bin

OBJS = build/kernel_entry.o build/interrupts.o build/kernel.o build/idt.o build/keyboard.o build/disk.o

all: $(IMG)

# Geração do Disco com Injeção Segura
$(IMG): $(BOOT_BIN) $(KERNEL_BIN) $(APP_BIN) scripts/mkfs.py
	mkdir -p build
	python3 scripts/mkfs.py
	# 1. Base (Boot + Kernel)
	cat $(BOOT_BIN) $(KERNEL_BIN) > build/temp.bin
	dd if=build/temp.bin of=$(IMG) bs=1474560 count=1 conv=sync
	# 2. Injeção do Diretório (Setor 100 - Seguro)
	dd if=build/directory.bin of=$(IMG) bs=512 seek=100 conv=notrunc
	# 3. Injeção de Dados (Setores 110, 111)
	dd if=build/readme.bin of=$(IMG) bs=512 seek=110 conv=notrunc
	dd if=build/secret.bin of=$(IMG) bs=512 seek=111 conv=notrunc
	# 4. Injeção do App (Setor 150)
	dd if=$(APP_BIN) of=$(IMG) bs=512 seek=150 conv=notrunc
	rm build/temp.bin

$(BOOT_BIN): src/boot/boot.asm src/boot/gdt.asm
	$(AS) -f bin src/boot/boot.asm -o $@

$(APP_BIN): src/apps/app.asm
	$(AS) -f bin src/apps/app.asm -o $@

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

build/kernel.o: src/kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): $(OBJS) scripts/kernel.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

clean: stop
	rm -rf build/*.o build/*.bin build/*.img

stop:
	-pkill -f qemu-system-i386

run-vnc: all
	@echo "Limpando processos antigos do QEMU..."
	-pkill -f qemu-system-i386
	@echo "Iniciando QEMU com HD IDE Segura..."
	qemu-system-i386 -hda $(IMG) -vnc :0 -display none &
