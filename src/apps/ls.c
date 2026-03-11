#include <api.h>

static int starts_with(const char* s, const char* prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) return 0;
    }
    return 1;
}

static int strcmp_eq(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *a == *b;
}

static void emit_unique(char seen[][16], int* seen_count, const char* name, int is_dir) {
    char short_name[16];
    int i = 0;

    while (name[i] != '\0' && name[i] != '/' && i < 15) {
        short_name[i] = name[i];
        i++;
    }
    short_name[i] = '\0';

    for (int j = 0; j < *seen_count; j++) {
        if (strcmp_eq(seen[j], short_name)) return;
    }

    for (int j = 0; j <= i; j++) seen[*seen_count][j] = short_name[j];
    (*seen_count)++;

    stdout_write("- ");
    stdout_write(short_name);
    if (is_dir || name[i] == '/') stdout_write("/");
    stdout_write("\n");
}

void main() {
    DirectoryEntry dir[DIRECTORY_ENTRY_COUNT];
    char seen[DIRECTORY_ENTRY_COUNT][16];
    int seen_count = 0;
    const char* cwd = ARG_BUFFER;
    const char* prefix = cwd;
    int prefix_len = 0;

    if (cwd[0] == '\0' || (cwd[0] == '/' && cwd[1] == '\0')) {
        prefix = "";
    } else if (cwd[0] == '/') {
        prefix = cwd + 1;
    }

    while (prefix[prefix_len] != '\0') prefix_len++;

    get_dir(dir);

    stdout_write("--- LISTAGEM DE DISCO (C-APP) ---\n");

    for(int i = 0; i < DIRECTORY_ENTRY_COUNT; i++) {
        if(dir[i].active) {
            const char* name = dir[i].name;

            if (prefix_len == 0) {
                emit_unique(seen, &seen_count, name, dir[i].is_executable == 2);
            } else {
                if (!starts_with(name, prefix)) continue;
                if (name[prefix_len] != '/') continue;
                emit_unique(seen, &seen_count, name + prefix_len + 1, dir[i].is_executable == 2);
            }
        }
    }
}
