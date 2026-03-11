; bash_entry.asm - Stack Reforçada
[bits 32]
extern main

global _start
_start:
    cli
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Configura stack de 64KB (longe do programa)
    mov esp, 0x50000 
    
    sti
    call main
    jmp $
