#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

extern void put_char(char c, uint8_t color);

void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    /* Mapeamento de scancodes para caracteres ASCII (Simplificado Set 1) */
    static char map[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
       'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
    };

    /* Se o bit 7 não estiver setado, a tecla foi pressionada */
    if (!(scancode & 0x80)) {
        if (scancode < sizeof(map)) {
            char key = map[scancode];
            if (key) {
                put_char(key, 15); /* Escreve em Branco no Terminal */
            }
        }
    }
}
