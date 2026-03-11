#include <api.h>
#include <app_utils.h>

void main() {
    char args[128];
    char* name;
    char* content;

    app_copy_args(args, sizeof(args));
    app_split_args(args, &name, &content);

    if (name[0] == '\0' || content[0] == '\0') {
        stdout_write("write: uso write <arquivo> <conteudo>\n");
        return;
    }

    if (!write_file(name, content)) {
        stdout_write("write: falha ao escrever arquivo\n");
    }
}
