#include <stdint.h>

/* Wrappers de Syscalls */
void sys_write(const char* s, int line, int col) {
    __asm__ __volatile__ ("int $0x80" : : "a"(1), "b"(s), "c"(line), "d"(col) : "memory");
}
char sys_read() {
    uint32_t c;
    __asm__ __volatile__ ("int $0x80" : "=a"(c) : "a"(2));
    return (char)c;
}
void sys_clear() { __asm__ __volatile__ ("int $0x80" : : "a"(3)); }
int sys_exec(const char* name) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(6), "b"(name));
    return (int)ret;
}

void run_loaded_app() {
    void (*app_entry)() = (void (*)())0x40000;
    app_entry();
}

/* Utils */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void main() {
    sys_clear();
    sys_write("BASH V5.6: COMANDOS ATIVOS", 0, 0);
    sys_write("root@colapso:# ", 2, 0);

    char cmd[64];
    int idx = 0;
    int col = 15;
    int line = 2;

    while (1) {
        char c = sys_read();
        if (c != 0) {
            if (c == '\n') {
                cmd[idx] = '\0';
                line++;
                
                if (strcmp(cmd, "clear") == 0) {
                    sys_clear();
                    line = 0;
                    sys_write("BASH V5.6: COMANDOS ATIVOS", 0, 0);
                } 
                else if (idx > 0) {
                    /* Tenta rodar bin/comando */
                    char b[32] = "bin/";
                    char* p = b + 4; char* s = cmd;
                    while((*p++ = *s++));
                    
                    if (!sys_exec(b)) {
                        sys_write("bash: comando nao encontrado", line++, 0);
                    } else {
                        run_loaded_app();
                        line++; /* Programa rodou e voltou */
                    }
                }

                if (line >= 23) { sys_clear(); line = 0; }
                sys_write("root@colapso:# ", line, 0);
                col = 15;
                idx = 0;
            } 
            else if (c == '\b') {
                if (idx > 0) {
                    idx--; col--;
                    sys_write(" ", line, col);
                }
            } 
            else if (idx < 60) {
                cmd[idx++] = c;
                char buf[2] = {c, '\0'};
                sys_write(buf, line, col++);
            }
        }
    }
}
