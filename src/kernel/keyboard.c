#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    static char map[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
       'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
    };

    if (!(scancode & 0x80)) {
        if (scancode < sizeof(map)) {
            char key = map[scancode];
            if (key) {
                /* Apenas deposita a tecla no buffer compartilhado */
                *(volatile char*)0x9000 = key;
            }
        }
    }
}
