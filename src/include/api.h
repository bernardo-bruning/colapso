#ifndef API_H
#define API_H

#include <stdint.h>
#include <disk.h>

/* Wrappers de Syscalls para C */
static inline void stdout_write(const char* s) {
    __asm__ __volatile__ ("int $0x80" : : "a"(1), "b"(s) : "memory");
}

static inline char stdin_read() {
    uint32_t c;
    __asm__ __volatile__ ("int $0x80" : "=a"(c) : "a"(2));
    return (char)c;
}

static inline void clear_screen() {
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

static inline int create_file(const char* name) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(11), "b"(name));
    return (int)ret;
}

static inline int write_file(const char* name, const char* content) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(12), "b"(name), "c"(content));
    return (int)ret;
}

static inline int create_dir(const char* name) {
    uint32_t ret;
    __asm__ __volatile__ ("int $0x80" : "=a"(ret) : "a"(13), "b"(name));
    return (int)ret;
}

#define ARG_BUFFER ((char*)0x9500)

#endif
