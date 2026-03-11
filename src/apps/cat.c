#include <api.h>

void main() {
    char buffer[512];
    const char* path = ARG_BUFFER;

    if (path[0] == '\0') {
        stdout_write("cat: informe um arquivo\n");
        return;
    }

    if (!read_file(path, buffer)) {
        stdout_write("cat: arquivo nao encontrado\n");
        return;
    }

    buffer[511] = '\0';
    stdout_write(buffer);

    /* Keep shell prompt on a fresh line when the file lacks a trailing LF. */
    {
        int len = 0;
        while (len < 511 && buffer[len] != '\0') len++;
        if (len == 0 || buffer[len - 1] != '\n') {
            stdout_write("\n");
        }
    }
}
