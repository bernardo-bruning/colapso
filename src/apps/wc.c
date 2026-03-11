#include <api.h>
#include <app_utils.h>

void main() {
    char buffer[512];
    char num[16];
    int lines = 0;
    int words = 0;
    int bytes = 0;
    int in_word = 0;

    if (ARG_BUFFER[0] == '\0') {
        stdout_write("wc: informe um arquivo\n");
        return;
    }

    if (!read_file(ARG_BUFFER, buffer)) {
        stdout_write("wc: arquivo nao encontrado\n");
        return;
    }

    for (int i = 0; i < 512 && buffer[i] != '\0'; i++) {
        char c = buffer[i];
        bytes++;
        if (c == '\n') lines++;

        if (c == ' ' || c == '\n' || c == '\t') {
            in_word = 0;
        } else if (!in_word) {
            words++;
            in_word = 1;
        }
    }

    app_u32_to_str((uint32_t)lines, num);
    stdout_write("linhas: ");
    stdout_write(num);
    stdout_write("\n");

    app_u32_to_str((uint32_t)words, num);
    stdout_write("palavras: ");
    stdout_write(num);
    stdout_write("\n");

    app_u32_to_str((uint32_t)bytes, num);
    stdout_write("bytes: ");
    stdout_write(num);
    stdout_write("\n");
}
