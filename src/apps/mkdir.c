#include <api.h>

void main() {
    if (ARG_BUFFER[0] == '\0') {
        stdout_write("mkdir: informe um nome\n");
        return;
    }

    if (!create_dir(ARG_BUFFER)) {
        stdout_write("mkdir: falha ao criar diretorio\n");
    }
}
