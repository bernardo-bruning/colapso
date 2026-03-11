; standalone.asm - Kernel Multiboot de Diagnóstico
[bits 32]

MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text
global _start
_start:
    ; 1. Limpa a tela VGA (0xB8000) com fundo azul
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov ax, 0x1F20 ; Espaço com fundo azul (1) e texto branco (F)
    rep stosw

    ; 2. Escreve "VIVA! COLAPSO ESTA VIVO" no centro
    mov esi, msg
    mov edi, 0xB8000 + (12 * 160) + 60 ; Linha 12, coluna 30
    mov ah, 0x1F ; Cor azul/branco
.loop:
    lodsb
    test al, al
    jz .serial
    stosw
    jmp .loop

.serial:
    ; 3. Escreve na porta Serial 0x3F8
    mov dx, 0x3f8
    mov al, 'V'
    out dx, al
    mov al, 'I'
    out dx, al
    mov al, 'V'
    out dx, al
    mov al, 'A'
    out dx, al
    mov al, '!'
    out dx, al

    ; 4. Hang
    cli
.hang:
    hlt
    jmp .hang

msg db "VIVA! COLAPSO ESTA VIVO", 0
