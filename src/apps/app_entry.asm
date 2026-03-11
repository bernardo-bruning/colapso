; app_entry.asm - Ponto de entrada generico para Apps
[bits 32]
extern main

global _start
_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, esp

    call main
    ret
