#include <api.h>

static void sys_shutdown(void) {
    __asm__ __volatile__ ("int $0x80" : : "a"(10));
}

void main() {
    stdout_write("shutdown...\n");
    sys_shutdown();
}
