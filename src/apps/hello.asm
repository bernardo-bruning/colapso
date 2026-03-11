; hello.asm - Programa Moderno com Syscall Exit
[bits 32]
[org 0x40000]

start:
    ; sys_write
    mov eax, 1
    mov ebx, msg
    mov ecx, 12
    mov edx, 25
    int 0x80

    ret

msg db "Hello World via Syscall Exit!", 0
