; app_entry.asm - Ponto de entrada generico para Apps
[bits 32]
extern main

global _start
section .bss
saved_esp resd 1
saved_ebp resd 1

section .text
_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov [saved_esp], esp
    mov [saved_ebp], ebp
    mov esp, 0x4F000
    mov ebp, esp

    call main
    mov esp, [saved_esp]
    mov ebp, [saved_ebp]
    ret
