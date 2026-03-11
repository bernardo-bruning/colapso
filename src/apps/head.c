#include <api.h>

void main() {
    char buffer[512];
    int lines = 0;

    if (ARG_BUFFER[0] == '\0') {
        stdout_write("head: informe um arquivo\n");
        return;
    }

    if (!read_file(ARG_BUFFER, buffer)) {
        stdout_write("head: arquivo nao encontrado\n");
        return;
    }

    for (int i = 0; i < 512 && buffer[i] != '\0'; i++) {
        char out[2] = {buffer[i], '\0'};
        stdout_write(out);
        if (buffer[i] == '\n' && ++lines >= 10) break;
    }
}
