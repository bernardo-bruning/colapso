#include <api.h>

static char buffer[4096];

static int sys_read_klog(char* buffer) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(8), "b"(buffer));
    return (int)ret;
}

void main() {
    buffer[0] = '\0';
    if (sys_read_klog(buffer)) {
        stdout_write(buffer);
        if (buffer[0] != '\0') stdout_write("\n");
    }
}
