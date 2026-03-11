#include <api.h>

void main() {
    if (ARG_BUFFER[0] != '\0') {
        stdout_write(ARG_BUFFER);
    }
    stdout_write("\n");
}
