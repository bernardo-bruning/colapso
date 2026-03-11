#include <api.h>

static void sys_reboot(void) {
    __asm__ __volatile__ ("int $0x80" : : "a"(9));
}

void main() {
    stdout_write("rebooting...\n");
    sys_reboot();
}
