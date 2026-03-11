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
HELP_BIN = build/help.bin
ECHO_BIN = build/echo.bin
STAT_BIN = build/stat.bin
HEXDUMP_BIN = build/hexdump.bin
UNAME_BIN = build/uname.bin
PWD_BIN = build/pwd.bin
WHICH_BIN = build/which.bin
WC_BIN = build/wc.bin
GREP_BIN = build/grep.bin
HEAD_BIN = build/head.bin
TAIL_BIN = build/tail.bin
MORE_BIN = build/more.bin
SLEEP_BIN = build/sleep.bin
DMESG_BIN = build/dmesg.bin
MEMINFO_BIN = build/meminfo.bin
REBOOT_BIN = build/reboot.bin
SHUTDOWN_BIN = build/shutdown.bin
TRUE_BIN = build/true.bin
FALSE_BIN = build/false.bin
CD_BIN = build/cd.bin
MKDIR_BIN = build/mkdir.bin
RM_BIN = build/rm.bin
CP_BIN = build/cp.bin
MV_BIN = build/mv.bin
TOUCH_BIN = build/touch.bin
WRITE_BIN = build/write.bin
DATE_BIN = build/date.bin
ENV_BIN = build/env.bin
EDIT_BIN = build/edit.bin

KERNEL_OBJS = build/kernel_entry.o build/interrupts.o build/kernel.o build/idt.o build/keyboard.o build/disk.o build/syscall.o
BASH_OBJS = build/bash_entry.o build/bash.o
APP_ENTRY = build/app_entry.o

all: $(IMG)

$(IMG): $(BOOT_BIN) $(KERNEL_BIN) $(BASH_BIN) $(LS_BIN) $(CAT_BIN) $(HELLO_BIN) $(HELP_BIN) $(ECHO_BIN) $(STAT_BIN) $(HEXDUMP_BIN) $(UNAME_BIN) $(PWD_BIN) $(WHICH_BIN) $(WC_BIN) $(GREP_BIN) $(HEAD_BIN) $(TAIL_BIN) $(MORE_BIN) $(SLEEP_BIN) $(DMESG_BIN) $(MEMINFO_BIN) $(REBOOT_BIN) $(SHUTDOWN_BIN) $(TRUE_BIN) $(FALSE_BIN) $(CD_BIN) $(MKDIR_BIN) $(RM_BIN) $(CP_BIN) $(MV_BIN) $(TOUCH_BIN) $(WRITE_BIN) $(DATE_BIN) $(ENV_BIN) $(EDIT_BIN) README.txt scripts/mkfs.py
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
	dd if=$(HELP_BIN) of=$(IMG) bs=512 seek=110 conv=notrunc
	dd if=$(ECHO_BIN) of=$(IMG) bs=512 seek=120 conv=notrunc
	dd if=$(STAT_BIN) of=$(IMG) bs=512 seek=130 conv=notrunc
	dd if=$(HEXDUMP_BIN) of=$(IMG) bs=512 seek=140 conv=notrunc
	dd if=$(UNAME_BIN) of=$(IMG) bs=512 seek=150 conv=notrunc
	dd if=$(PWD_BIN) of=$(IMG) bs=512 seek=160 conv=notrunc
	dd if=$(WHICH_BIN) of=$(IMG) bs=512 seek=170 conv=notrunc
	dd if=$(WC_BIN) of=$(IMG) bs=512 seek=180 conv=notrunc
	dd if=$(GREP_BIN) of=$(IMG) bs=512 seek=190 conv=notrunc
	dd if=$(HEAD_BIN) of=$(IMG) bs=512 seek=210 conv=notrunc
	dd if=$(TAIL_BIN) of=$(IMG) bs=512 seek=220 conv=notrunc
	dd if=$(MORE_BIN) of=$(IMG) bs=512 seek=230 conv=notrunc
	dd if=$(SLEEP_BIN) of=$(IMG) bs=512 seek=240 conv=notrunc
	dd if=$(DMESG_BIN) of=$(IMG) bs=512 seek=250 conv=notrunc
	dd if=$(MEMINFO_BIN) of=$(IMG) bs=512 seek=260 conv=notrunc
	dd if=$(REBOOT_BIN) of=$(IMG) bs=512 seek=270 conv=notrunc
	dd if=$(SHUTDOWN_BIN) of=$(IMG) bs=512 seek=280 conv=notrunc
	dd if=$(TRUE_BIN) of=$(IMG) bs=512 seek=290 conv=notrunc
	dd if=$(FALSE_BIN) of=$(IMG) bs=512 seek=300 conv=notrunc
	dd if=$(CD_BIN) of=$(IMG) bs=512 seek=310 conv=notrunc
	dd if=$(MKDIR_BIN) of=$(IMG) bs=512 seek=320 conv=notrunc
	dd if=$(RM_BIN) of=$(IMG) bs=512 seek=330 conv=notrunc
	dd if=$(CP_BIN) of=$(IMG) bs=512 seek=340 conv=notrunc
	dd if=$(MV_BIN) of=$(IMG) bs=512 seek=350 conv=notrunc
	dd if=$(TOUCH_BIN) of=$(IMG) bs=512 seek=360 conv=notrunc
	dd if=$(WRITE_BIN) of=$(IMG) bs=512 seek=370 conv=notrunc
	dd if=$(DATE_BIN) of=$(IMG) bs=512 seek=380 conv=notrunc
	dd if=$(ENV_BIN) of=$(IMG) bs=512 seek=390 conv=notrunc
	dd if=$(EDIT_BIN) of=$(IMG) bs=512 seek=400 conv=notrunc
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

build/help.o: src/apps/help.c
	$(CC) $(CFLAGS) -c $< -o $@

$(HELP_BIN): $(APP_ENTRY) build/help.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/help.o -o $@

build/echo.o: src/apps/echo.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ECHO_BIN): $(APP_ENTRY) build/echo.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/echo.o -o $@

build/stat.o: src/apps/stat.c
	$(CC) $(CFLAGS) -c $< -o $@

$(STAT_BIN): $(APP_ENTRY) build/stat.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/stat.o -o $@

build/hexdump.o: src/apps/hexdump.c
	$(CC) $(CFLAGS) -c $< -o $@

$(HEXDUMP_BIN): $(APP_ENTRY) build/hexdump.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/hexdump.o -o $@

build/uname.o: src/apps/uname.c
	$(CC) $(CFLAGS) -c $< -o $@

$(UNAME_BIN): $(APP_ENTRY) build/uname.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/uname.o -o $@

build/pwd.o: src/apps/pwd.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PWD_BIN): $(APP_ENTRY) build/pwd.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/pwd.o -o $@

build/which.o: src/apps/which.c
	$(CC) $(CFLAGS) -c $< -o $@

$(WHICH_BIN): $(APP_ENTRY) build/which.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/which.o -o $@

build/wc.o: src/apps/wc.c
	$(CC) $(CFLAGS) -c $< -o $@

$(WC_BIN): $(APP_ENTRY) build/wc.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/wc.o -o $@

build/grep.o: src/apps/grep.c
	$(CC) $(CFLAGS) -c $< -o $@

$(GREP_BIN): $(APP_ENTRY) build/grep.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/grep.o -o $@

build/head.o: src/apps/head.c
	$(CC) $(CFLAGS) -c $< -o $@

$(HEAD_BIN): $(APP_ENTRY) build/head.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/head.o -o $@

build/tail.o: src/apps/tail.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TAIL_BIN): $(APP_ENTRY) build/tail.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/tail.o -o $@

build/more.o: src/apps/more.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MORE_BIN): $(APP_ENTRY) build/more.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/more.o -o $@

build/sleep.o: src/apps/sleep.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SLEEP_BIN): $(APP_ENTRY) build/sleep.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/sleep.o -o $@

build/dmesg.o: src/apps/dmesg.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DMESG_BIN): $(APP_ENTRY) build/dmesg.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/dmesg.o -o $@

build/meminfo.o: src/apps/meminfo.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MEMINFO_BIN): $(APP_ENTRY) build/meminfo.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/meminfo.o -o $@

build/reboot.o: src/apps/reboot.c
	$(CC) $(CFLAGS) -c $< -o $@

$(REBOOT_BIN): $(APP_ENTRY) build/reboot.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/reboot.o -o $@

build/shutdown.o: src/apps/shutdown.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SHUTDOWN_BIN): $(APP_ENTRY) build/shutdown.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/shutdown.o -o $@

build/true.o: src/apps/true.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TRUE_BIN): $(APP_ENTRY) build/true.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/true.o -o $@

build/false.o: src/apps/false.c
	$(CC) $(CFLAGS) -c $< -o $@

$(FALSE_BIN): $(APP_ENTRY) build/false.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/false.o -o $@

build/cd.o: src/apps/cd.c
	$(CC) $(CFLAGS) -c $< -o $@

$(CD_BIN): $(APP_ENTRY) build/cd.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/cd.o -o $@

build/mkdir.o: src/apps/mkdir.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MKDIR_BIN): $(APP_ENTRY) build/mkdir.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/mkdir.o -o $@

build/rm.o: src/apps/rm.c
	$(CC) $(CFLAGS) -c $< -o $@

$(RM_BIN): $(APP_ENTRY) build/rm.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/rm.o -o $@

build/cp.o: src/apps/cp.c
	$(CC) $(CFLAGS) -c $< -o $@

$(CP_BIN): $(APP_ENTRY) build/cp.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/cp.o -o $@

build/mv.o: src/apps/mv.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MV_BIN): $(APP_ENTRY) build/mv.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/mv.o -o $@

build/touch.o: src/apps/touch.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TOUCH_BIN): $(APP_ENTRY) build/touch.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/touch.o -o $@

build/write.o: src/apps/write.c
	$(CC) $(CFLAGS) -c $< -o $@

$(WRITE_BIN): $(APP_ENTRY) build/write.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/write.o -o $@

build/date.o: src/apps/date.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DATE_BIN): $(APP_ENTRY) build/date.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/date.o -o $@

build/env.o: src/apps/env.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ENV_BIN): $(APP_ENTRY) build/env.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/env.o -o $@

build/edit.o: src/apps/edit.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EDIT_BIN): $(APP_ENTRY) build/edit.o scripts/app.ld
	$(LD) -m elf_i386 -T scripts/app.ld $(APP_ENTRY) build/edit.o -o $@

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
	python3 tests/validate_utils.py
