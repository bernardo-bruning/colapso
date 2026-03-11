/* kernel.c - O Coração do Colapso OS */
#include <stdint.h>

#define VGA_ADDRESS 0xB8000
#define BUFSIZE 2000

/* Cores VGA */
enum vga_color {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
    RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GREY = 7,
    DARK_GREY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10,
    LIGHT_CYAN = 11, LIGHT_RED = 12, LIGHT_MAGENTA = 13,
    LIGHT_BROWN = 14, WHITE = 15,
};

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void clear_screen(uint8_t color) {
    uint16_t* terminal_buffer = (uint16_t*) VGA_ADDRESS;
    for (int i = 0; i < BUFSIZE; i++) {
        terminal_buffer[i] = vga_entry(' ', color);
    }
}

void kprint(const char* data, uint8_t color, int line, int col) {
    uint16_t* terminal_buffer = (uint16_t*) VGA_ADDRESS;
    int index = (line * 80) + col;
    for (int i = 0; data[i] != '\0'; i++) {
        terminal_buffer[index++] = vga_entry(data[i], color);
    }
}

void kernel_main(void) {
    /* 1. Limpa a tela com fundo azul escuro */
    clear_screen(BLUE);

    /* 2. Cabeçalho do Sistema */
    kprint("--------------------------------------------------------------------------------", WHITE, 0, 0);
    kprint("                           COLAPSO OPERATING SYSTEM                             ", LIGHT_CYAN, 1, 0);
    kprint("--------------------------------------------------------------------------------", WHITE, 2, 0);

    /* 3. Status do Kernel */
    kprint("[ OK ] Kernel carregado em 0x1000", LIGHT_GREEN, 4, 2);
    kprint("[ OK ] Modo Protegido (32-bit) Ativado", LIGHT_GREEN, 5, 2);
    kprint("[ OK ] GDT Inicializada", LIGHT_GREEN, 6, 2);
    
    kprint("Bem-vindo ao nucleo do sistema. O hardware esta sob nosso controle.", WHITE, 8, 2);

    /* 4. Rodapé */
    kprint("Status: Kernel Space Active | Arq: x86_32", LIGHT_GREY, 24, 0);

    while(1);
}
