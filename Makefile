# Makefile para o Colapso OS (Floppy Infalível)

AS = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -Isrc/include -fno-pic -fno-stack-protector
LDFLAGS = -m elf_i386 -T scripts/kernel.ld

IMG = build/colapso.img
KERNEL_BIN = build/kernel.bin
BOOT_BIN = build/boot.bin

all: $(IMG)

$(BOOT_BIN): src/boot/boot.asm src/boot/gdt.asm
	mkdir -p build
	$(AS) -f bin src/boot/boot.asm -o $@

build/kernel_entry.o: src/kernel/kernel_entry.asm
	$(AS) -f elf32 src/kernel/kernel_entry.asm -o $@

build/kernel.o: src/kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BIN): build/kernel_entry.o build/kernel.o scripts/kernel.ld
	$(LD) $(LDFLAGS) build/kernel_entry.o build/kernel.o -o $@

$(IMG): $(BOOT_BIN) $(KERNEL_BIN)
	# Junta Bootloader e Kernel
	cat $(BOOT_BIN) $(KERNEL_BIN) > build/temp.bin
	# Cria a imagem de Floppy de 1.44MB (infalível para BIOS)
	dd if=build/temp.bin of=$(IMG) bs=1474560 count=1 conv=sync
	rm build/temp.bin

clean:
	rm -rf build/*.o build/*.bin $(IMG)
	-pkill -f qemu-system-i386

run: $(IMG)
	qemu-system-i386 -fda $(IMG) -serial stdio

run-vnc: $(IMG)
	@echo "Limpando processos antigos do QEMU..."
	-pkill -f qemu-system-i386
	@echo "Iniciando QEMU com VNC na porta 5900 (Floppy Boot)..."
	qemu-system-i386 -fda $(IMG) -vnc :0 -display none &
	@echo "-------------------------------------------------------"
	@echo "SUCESSO: Colapso OS rodando via Floppy Boot."
	@echo "Conecte seu cliente VNC em localhost:5900"
	@echo "-------------------------------------------------------"

stop:
	-pkill -f qemu-system-i386
