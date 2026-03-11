#ifndef API_H
#define API_H

#include <stdint.h>

typedef struct {
    char name[16];
    uint32_t start_lba;
    uint32_t sector_count;
    uint32_t is_executable;
    uint32_t active;
} DirectoryEntry;

/* Wrappers de Syscalls para C */
static inline void print(const char* s, int line, int col) {
    __asm__ __volatile__ ("int $0x80" : : "a"(1), "b"(s), "c"(line), "d"(col) : "memory");
}

static inline char read_key() {
    uint32_t c;
    __asm__ __volatile__ ("int $0x80" : "=a"(c) : "a"(2));
    return (char)c;
}

static inline void clear() {
    __asm__ __volatile__ ("int $0x80" : : "a"(3));
}

static inline void get_dir(DirectoryEntry* dir) {
    __asm__ __volatile__ ("int $0x80" : : "a"(4), "b"(dir));
}

static inline int read_file(const char* name, char* buffer) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(5), "b"(name), "c"(buffer));
    return (int)ret;
}

static inline void exit_app() {
    __asm__ __volatile__ ("int $0x80" : : "a"(7));
}

/* Memória de Argumentos (Passados pelo Bash) */
#define ARG_BUFFER ((char*)0x9500)

#endif
