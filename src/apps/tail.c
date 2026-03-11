#include <api.h>

void main() {
    char buffer[512];
    int line_starts[64];
    int line_count = 0;
    int start_line = 0;

    if (ARG_BUFFER[0] == '\0') {
        stdout_write("tail: informe um arquivo\n");
        return;
    }

    if (!read_file(ARG_BUFFER, buffer)) {
        stdout_write("tail: arquivo nao encontrado\n");
        return;
    }

    line_starts[line_count++] = 0;
    for (int i = 0; i < 512 && buffer[i] != '\0'; i++) {
        if (buffer[i] == '\n' && line_count < 64 && buffer[i + 1] != '\0') {
            line_starts[line_count++] = i + 1;
        }
    }

    if (line_count > 10) start_line = line_count - 10;

    for (int i = line_starts[start_line]; i < 512 && buffer[i] != '\0'; i++) {
        char out[2] = {buffer[i], '\0'};
        stdout_write(out);
    }
}
