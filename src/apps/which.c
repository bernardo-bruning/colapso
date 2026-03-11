#include <api.h>
#include <app_utils.h>

void main() {
    DirectoryEntry dir[DIRECTORY_ENTRY_COUNT];
    char path[32] = "bin/";
    int offset = 4;

    if (ARG_BUFFER[0] == '\0') {
        stdout_write("which: informe um comando\n");
        return;
    }

    while (ARG_BUFFER[offset - 4] != '\0' && offset < 31) {
        path[offset] = ARG_BUFFER[offset - 4];
        offset++;
    }
    path[offset] = '\0';

    if (app_str_eq(ARG_BUFFER, "clear")) {
        stdout_write("clear: shell builtin\n");
        return;
    }

    get_dir(dir);
    for (int i = 0; i < DIRECTORY_ENTRY_COUNT; i++) {
        if (dir[i].active && app_str_eq(dir[i].name, path)) {
            stdout_write(path);
            stdout_write("\n");
            return;
        }
    }

    stdout_write("which: comando nao encontrado\n");
}
