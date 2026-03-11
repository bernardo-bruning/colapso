; hello.asm - Programa Moderno com Syscall Exit
[bits 32]
[org 0x40000]

start:
    ; sys_stdout_write
    mov eax, 1
    mov ebx, msg
    int 0x80

    ret

msg db "Hello World via Syscall Exit!", 10, 0
