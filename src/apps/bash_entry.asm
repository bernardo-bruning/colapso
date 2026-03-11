; bash_entry.asm - Ponto de Entrada Seguro
[bits 32]
extern main

global _start
_start:
    cli
    ; Configura segmentos de dados
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Configura stack IMEDIATAMENTE antes do programa (0x20000)
    mov esp, 0x1FFFF
    mov ebp, esp

    sti
    call main
    
    ; Caso o main retorne
    cli
    hlt
