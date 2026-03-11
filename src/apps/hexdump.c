#include <api.h>

static void byte_to_hex(unsigned char value, char* out) {
    static const char hex[] = "0123456789ABCDEF";
    out[0] = hex[(value >> 4) & 0x0F];
    out[1] = hex[value & 0x0F];
    out[2] = '\0';
}

void main() {
    char buffer[512];
    char hex[3];
    int bytes_printed = 0;

    if (ARG_BUFFER[0] == '\0') {
        stdout_write("hexdump: informe um arquivo\n");
        return;
    }

    if (!read_file(ARG_BUFFER, buffer)) {
        stdout_write("hexdump: arquivo nao encontrado\n");
        return;
    }

    for (int i = 0; i < 512 && buffer[i] != '\0'; i++) {
        byte_to_hex((unsigned char)buffer[i], hex);
        stdout_write(hex);
        bytes_printed++;

        if ((bytes_printed % 16) == 0) {
            stdout_write("\n");
        } else {
            stdout_write(" ");
        }
    }

    if ((bytes_printed % 16) != 0) {
        stdout_write("\n");
    }
}
