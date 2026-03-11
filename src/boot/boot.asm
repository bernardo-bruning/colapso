; boot.asm - O Bootloader Infalível
[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000

start:
    ; 1. Normalizar Segmentos
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    mov [BOOT_DRIVE], dl

    ; 2. Limpar a tela (BIOS)
    mov ax, 0x0003
    int 0x10

    ; 3. Carregar o Kernel do Disquete (Setor 2)
    mov bx, KERNEL_OFFSET
    mov ah, 0x02
    mov al, 15 ; Ler 15 setores
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc disk_error

    ; 4. Ativar A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 5. Modo Protegido
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

%include "src/boot/gdt.asm"

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    call KERNEL_OFFSET ; PULA PARA O KERNEL FLAT BINARY
    jmp $

disk_error:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $

BOOT_DRIVE db 0
times 510-($-$$) db 0
dw 0xAA55
