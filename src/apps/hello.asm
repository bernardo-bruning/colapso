; hello.asm - Programa Hello World via Syscall
[bits 32]
[org 0x40000] ; O Kernel carrega apps em 0x40000

start:
    ; Configura parâmetros para sys_write (EAX=1)
    mov eax, 1          ; Syscall 1
    mov ebx, msg        ; EBX = String
    mov ecx, 12         ; ECX = Linha 12
    mov edx, 30         ; EDX = Coluna 30
    int 0x80            ; Chama o Kernel!

    ret                 ; Retorna para o Kernel/Bash

msg db "Hello from Colapso OS!", 0
