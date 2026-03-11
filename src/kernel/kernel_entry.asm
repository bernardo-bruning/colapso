; kernel_entry.asm - Garante que chamamos o kernel_main corretamente
[bits 32]
extern kernel_main
call kernel_main
jmp $ ; Loop infinito caso o kernel retorne
