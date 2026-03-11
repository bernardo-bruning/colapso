; bash_entry.asm - Ambiente Limpo para o Bash
[bits 32]
extern main

global _start
_start:
    cli

    ; Inicializa Segmentos de Dados do Modo Protegido (0x10 é o DATA_SEG na GDT)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Sinal Visual: Roxo na tela inteira para provar que o ASM rodou
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x5F20
    rep stosw

    ; Configura stack segura (0x10000)
    mov esp, 0x10000 
    
    sti ; Ativa interrupções para o teclado
    call main
    
    jmp $
