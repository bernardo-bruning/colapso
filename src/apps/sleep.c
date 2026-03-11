#include <api.h>
#include <app_utils.h>

static uint32_t parse_u32(const char* s) {
    uint32_t value = 0;
    for (int i = 0; s[i] >= '0' && s[i] <= '9'; i++) {
        value = (value * 10) + (uint32_t)(s[i] - '0');
    }
    return value;
}

void main() {
    volatile uint32_t delay;
    uint32_t units;

    if (ARG_BUFFER[0] == '\0') {
        stdout_write("sleep: informe um numero\n");
        return;
    }

    units = parse_u32(ARG_BUFFER);
    for (uint32_t i = 0; i < units; i++) {
        for (delay = 0; delay < 5000000; delay++) {}
    }
}
