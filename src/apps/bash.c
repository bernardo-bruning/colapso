#include <stdint.h>

/* Estrutura de diretório para o Bash */
typedef struct {
    char name[16];
    uint32_t start_lba;
    uint32_t sector_count;
    uint32_t is_executable;
    uint32_t active;
} DirectoryEntry;

/* Wrappers de Syscalls */
void sys_write(const char* s, int line, int col) {
    __asm__ __volatile__ ("int $0x80" : : "a"(1), "b"(s), "c"(line), "d"(col) : "memory");
}

char sys_read() {
    uint32_t c;
    __asm__ __volatile__ ("int $0x80" : "=a"(c) : "a"(2));
    return (char)c;
}

void sys_clear() {
    __asm__ __volatile__ ("int $0x80" : : "a"(3));
}

void sys_get_dir(DirectoryEntry* dir) {
    __asm__ __volatile__ ("int $0x80" : : "a"(4), "b"(dir));
}

int sys_read_file(const char* name, char* buffer) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(5), "b"(name), "c"(buffer));
    return (int)ret;
}

/* Funções utilitárias */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n--) {
        if (*s1 != *s2) return *(unsigned char*)s1 - *(unsigned char*)s2;
        if (*s1 == 0) break;
        s1++; s2++;
    }
    return 0;
}

void main() {
    sys_clear();
    sys_write("--- COLAPSO BASH V4.0 (FULL SYSCALL INTERACTIVE) ---", 0, 15);
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
                
                if (strcmp(cmd, "help") == 0) {
                    sys_write("Comandos: help, ls, cat <file>, clear", line++, 0);
                }
                else if (strcmp(cmd, "ls") == 0) {
                    DirectoryEntry dir[16];
                    sys_get_dir(dir);
                    sys_write("Arquivos no disco:", line++, 0);
                    for(int i=0; i<16; i++) {
                        if(dir[i].active) {
                            sys_write("- ", line, 2);
                            sys_write(dir[i].name, line++, 4);
                        }
                    }
                }
                else if (strncmp(cmd, "cat ", 4) == 0) {
                    char buf[512];
                    if (sys_read_file(cmd + 4, buf)) {
                        sys_write(buf, line++, 0);
                    } else {
                        sys_write("Erro: Arquivo nao encontrado.", line++, 0);
                    }
                }
                else if (strcmp(cmd, "clear") == 0) {
                    sys_clear();
                    line = 0;
                    sys_write("--- COLAPSO BASH V4.0 (FULL SYSCALL INTERACTIVE) ---", 0, 15);
                }
                else if (idx > 0) {
                    sys_write("bash: comando nao encontrado", line++, 0);
                }

                if (line >= 24) { sys_clear(); line = 0; }
                sys_write("root@colapso:# ", line, 0);
                col = 15; idx = 0;
            } 
            else if (c == '\b') {
                if (idx > 0) {
                    idx--; col--;
                    sys_write(" ", line, col);
                }
            } 
            else if (idx < 60) {
                cmd[idx++] = c;
                char b[2] = {c, 0};
                sys_write(b, line, col++);
            }
        }
    }
}
