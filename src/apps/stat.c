#include <api.h>

static char lower_ascii(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + ('a' - 'A'));
    return c;
}

static int strcmp_casefold(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = lower_ascii(*s1);
        char c2 = lower_ascii(*s2);
        if (c1 != c2) return (unsigned char)c1 - (unsigned char)c2;
        s1++;
        s2++;
    }
    return (unsigned char)lower_ascii(*s1) - (unsigned char)lower_ascii(*s2);
}

static void u32_to_str(uint32_t value, char* out) {
    char tmp[16];
    int idx = 0;

    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while (value > 0) {
        tmp[idx++] = (char)('0' + (value % 10));
        value /= 10;
    }

    for (int i = 0; i < idx; i++) {
        out[i] = tmp[idx - 1 - i];
    }
    out[idx] = '\0';
}

void main() {
    DirectoryEntry dir[DIRECTORY_ENTRY_COUNT];
    char num[16];
    const char* path = ARG_BUFFER;

    if (path[0] == '/') path++;

    if (path[0] == '\0') {
        stdout_write("stat: informe um arquivo\n");
        return;
    }

    get_dir(dir);
    for (int i = 0; i < DIRECTORY_ENTRY_COUNT; i++) {
        if (dir[i].active && strcmp_casefold(dir[i].name, path) == 0) {
            stdout_write("nome: ");
            stdout_write(dir[i].name);
            stdout_write("\n");

            stdout_write("lba: ");
            u32_to_str(dir[i].start_lba, num);
            stdout_write(num);
            stdout_write("\n");

            stdout_write("setores: ");
            u32_to_str(dir[i].sector_count, num);
            stdout_write(num);
            stdout_write("\n");

            stdout_write("executavel: ");
            u32_to_str(dir[i].is_executable, num);
            stdout_write(num);
            stdout_write("\n");
            return;
        }
    }

    stdout_write("stat: arquivo nao encontrado\n");
}
