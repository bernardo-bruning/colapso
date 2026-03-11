#include <api.h>

void main() {
    if (ARG_BUFFER[0] == '\0') {
        stdout_write("touch: informe um arquivo\n");
        return;
    }

    if (!create_file(ARG_BUFFER)) {
        stdout_write("touch: falha ao criar arquivo\n");
    }
}
