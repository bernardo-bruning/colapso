#include <api.h>
#include <app_utils.h>

void main() {
    char args[128];
    char buffer[512];
    char* pattern;
    char* file;
    char line[128];
    int line_idx = 0;

    app_copy_args(args, sizeof(args));
    app_split_args(args, &pattern, &file);

    if (pattern[0] == '\0' || file[0] == '\0') {
        stdout_write("grep: uso grep <padrao> <arquivo>\n");
        return;
    }

    if (!read_file(file, buffer)) {
        stdout_write("grep: arquivo nao encontrado\n");
        return;
    }

    for (int i = 0; i < 512 && buffer[i] != '\0'; i++) {
        char c = buffer[i];
        if (c == '\n' || line_idx == 127) {
            line[line_idx] = '\0';
            if (app_contains(line, pattern)) {
                stdout_write(line);
                stdout_write("\n");
            }
            line_idx = 0;
        } else {
            line[line_idx++] = c;
        }
    }

    if (line_idx > 0) {
        line[line_idx] = '\0';
        if (app_contains(line, pattern)) {
            stdout_write(line);
            stdout_write("\n");
        }
    }
}
