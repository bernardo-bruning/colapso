; app.asm - O Primeiro Aplicativo Externo do Colapso OS
[bits 32]
[org 0x20000] ; Vamos carregar este programa no endereço 0x20000 na RAM

start:
    ; Escreve "DISCO!" no canto inferior direito da tela (Linha 23, Coluna 60)
    mov edi, 0xB8000 + (23 * 160) + 120
    mov esi, msg
    mov ah, 0x4F ; Fundo Vermelho, Letra Branca
.loop:
    lodsb
    test al, al
    jz .done
    stosw
    jmp .loop
.done:
    ret ; Retorna o controle para o Kernel

msg db "APP LIDO DO DISCO LBA 50!", 0
