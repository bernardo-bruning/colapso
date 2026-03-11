#include <api.h>

void run_loaded_app() {
    void (*app_entry)() = (void (*)())0x40000;
    app_entry();
}

/* Utils */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static int sys_exec(const char* name) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(6), "b"(name));
    return (int)ret;
}

static void clear_args(void) {
    for (int i = 0; i < 128; i++) ARG_BUFFER[i] = '\0';
}

static void set_args(const char* args) {
    int i = 0;
    while (args[i] != '\0' && i < 127) {
        ARG_BUFFER[i] = args[i];
        i++;
    }
    ARG_BUFFER[i] = '\0';
}

static void print_prompt(void) {
    stdout_write("root@colapso:# ");
}

void main() {
    clear_screen();
    stdout_write("BASH V5.6: COMANDOS ATIVOS\n");
    print_prompt();

    char cmd[64];
    int idx = 0;

    while (1) {
        char c = stdin_read();
        if (c != 0) {
            if (c == '\n') {
                cmd[idx] = '\0';
                stdout_write("\n");
                
                if (strcmp(cmd, "clear") == 0) {
                    clear_screen();
                    stdout_write("BASH V5.6: COMANDOS ATIVOS\n");
                } 
                else if (idx > 0) {
                    char* args = cmd;
                    char b[32] = "bin/";
                    char* p = b + 4;
                    while (*args != '\0' && *args != ' ') {
                        *p++ = *args++;
                    }
                    *p = '\0';

                    if (*args == ' ') {
                        while (*args == ' ') args++;
                        set_args(args);
                    } else {
                        clear_args();
                    }
                    
                    if (!sys_exec(b)) {
                        stdout_write("bash: comando nao encontrado\n");
                    } else {
                        run_loaded_app();
                    }
                }

                print_prompt();
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
                char buf[2] = {c, '\0'};
                stdout_write(buf);
            }
        }
    }
}
