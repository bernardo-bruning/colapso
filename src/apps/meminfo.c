#include <api.h>

void main() {
    stdout_write("kernel: 0x00001000\n");
    stdout_write("bash: 0x00020000\n");
    stdout_write("app: 0x00040000\n");
    stdout_write("kbd buffer: 0x00009000\n");
    stdout_write("arg buffer: 0x00009500\n");
    stdout_write("vga: 0x000B8000\n");
}
