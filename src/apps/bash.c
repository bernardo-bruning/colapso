#include <api.h>

void run_loaded_app() {
    void (*app_entry)() = (void (*)())0x40000;
    app_entry();
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static int str_len(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

static void copy_str(char* dst, const char* src) {
    while ((*dst++ = *src++));
}

static int sys_exec(const char* name) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(6), "b"(name));
    return (int)ret;
}

static void set_args(const char* args) {
    int i = 0;
    while (args[i] != '\0' && i < 127) {
        ARG_BUFFER[i] = args[i];
        i++;
    }
    ARG_BUFFER[i] = '\0';
}

static void parent_dir(char* cwd) {
    int len = str_len(cwd);
    if (len <= 1) {
        cwd[0] = '/';
        cwd[1] = '\0';
        return;
    }

    while (len > 1 && cwd[len - 1] != '/') len--;
    if (len <= 1) {
        cwd[0] = '/';
        cwd[1] = '\0';
    } else {
        cwd[len - 1] = '\0';
    }
}

static void resolve_path(const char* cwd, const char* input, char* out) {
    if (input[0] == '\0' || strcmp(input, ".") == 0) {
        copy_str(out, cwd);
        return;
    }

    if (strcmp(input, "..") == 0) {
        copy_str(out, cwd);
        parent_dir(out);
        return;
    }

    if (input[0] == '/') {
        copy_str(out, input);
        return;
    }

    if (cwd[0] == '/' && cwd[1] == '\0') {
        out[0] = '/';
        out[1] = '\0';
        copy_str(out + 1, input);
        return;
    }

    copy_str(out, cwd);
    copy_str(out + str_len(out), "/");
    copy_str(out + str_len(out), input);
}

static int is_directory_path(const char* path) {
    DirectoryEntry dir[DIRECTORY_ENTRY_COUNT];
    const char* name = path[0] == '/' ? path + 1 : path;

    if (name[0] == '\0') return 1;

    get_dir(dir);
    for (int i = 0; i < DIRECTORY_ENTRY_COUNT; i++) {
        if (dir[i].active && strcmp(dir[i].name, name) == 0 && dir[i].is_executable == 2) {
            return 1;
        }
    }
    return 0;
}

static void print_prompt(const char* cwd) {
    stdout_write("root@colapso:");
    stdout_write(cwd);
    stdout_write("# ");
}

void main() {
    char cwd[64] = "/";
    char cmd[64];
    int idx = 0;

    clear_screen();
    stdout_write("BASH 0.1: COMANDOS ATIVOS\n");
    print_prompt(cwd);

    while (1) {
        char c = stdin_read();
        if (c == 0) continue;

        if (c == '\n') {
            char command[32];
            char* rest = cmd;
            char* p = command;

            cmd[idx] = '\0';
            stdout_write("\n");

            while (*rest != '\0' && *rest != ' ' && (p - command) < 31) {
                *p++ = *rest++;
            }
            *p = '\0';
            while (*rest == ' ') rest++;

            if (strcmp(command, "clear") == 0) {
                clear_screen();
                stdout_write("BASH 0.1: COMANDOS ATIVOS\n");
            }
            else if (strcmp(command, "cd") == 0) {
                char path[64];
                if (*rest == '\0') {
                    copy_str(cwd, "/");
                } else {
                    resolve_path(cwd, rest, path);
                    if (is_directory_path(path)) {
                        copy_str(cwd, path);
                    } else {
                        stdout_write("cd: diretorio nao encontrado\n");
                    }
                }
            }
            else if (idx > 0) {
                char exec_path[32] = "bin/";
                char resolved[128];

                copy_str(exec_path + 4, command);

                if (strcmp(command, "pwd") == 0) {
                    set_args(cwd);
                }
                else if (strcmp(command, "ls") == 0) {
                    char path[64];
                    resolve_path(cwd, rest, path);
                    set_args(path);
                }
                else if (
                    strcmp(command, "cat") == 0 ||
                    strcmp(command, "stat") == 0 ||
                    strcmp(command, "hexdump") == 0 ||
                    strcmp(command, "head") == 0 ||
                    strcmp(command, "tail") == 0 ||
                    strcmp(command, "more") == 0 ||
                    strcmp(command, "wc") == 0 ||
                    strcmp(command, "touch") == 0 ||
                    strcmp(command, "mkdir") == 0
                ) {
                    char path[64];
                    resolve_path(cwd, rest, path);
                    set_args(path);
                }
                else if (strcmp(command, "write") == 0) {
                    char path[64];
                    char* content = rest;
                    while (*content != '\0' && *content != ' ') content++;
                    if (*content != '\0') *content++ = '\0';
                    while (*content == ' ') content++;
                    resolve_path(cwd, rest, path);
                    copy_str(resolved, path);
                    copy_str(resolved + str_len(resolved), " ");
                    copy_str(resolved + str_len(resolved), content);
                    set_args(resolved);
                }
                else if (strcmp(command, "grep") == 0) {
                    char* file = rest;
                    char path[64];
                    while (*file != '\0' && *file != ' ') file++;
                    if (*file != '\0') *file++ = '\0';
                    while (*file == ' ') file++;
                    resolve_path(cwd, file, path);
                    copy_str(resolved, rest);
                    copy_str(resolved + str_len(resolved), " ");
                    copy_str(resolved + str_len(resolved), path);
                    set_args(resolved);
                }
                else {
                    set_args(rest);
                }

                if (!sys_exec(exec_path)) {
                    stdout_write("bash: comando nao encontrado\n");
                } else {
                    run_loaded_app();
                }
            }

            print_prompt(cwd);
            idx = 0;
        }
        else if (c == '\b') {
            if (idx > 0) {
                idx--;
                stdout_write("\b \b");
            }
        }
        else if (idx < 60) {
            cmd[idx++] = c;
            {
                char buf[2] = {c, '\0'};
                stdout_write(buf);
            }
        }
    }
}
