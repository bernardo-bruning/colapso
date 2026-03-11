#ifndef APP_UTILS_H
#define APP_UTILS_H

#include <api.h>

static inline char app_lower_ascii(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + ('a' - 'A'));
    return c;
}

static inline int app_str_eq(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) return 0;
        s1++;
        s2++;
    }
    return *s1 == '\0' && *s2 == '\0';
}

static inline int app_str_eq_casefold(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (app_lower_ascii(*s1) != app_lower_ascii(*s2)) return 0;
        s1++;
        s2++;
    }
    return *s1 == '\0' && *s2 == '\0';
}

static inline int app_strlen(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

static inline int app_contains(const char* haystack, const char* needle) {
    int needle_len = app_strlen(needle);
    if (needle_len == 0) return 1;
    for (int i = 0; haystack[i] != '\0'; i++) {
        int j = 0;
        while (needle[j] != '\0' && haystack[i + j] == needle[j]) j++;
        if (j == needle_len) return 1;
    }
    return 0;
}

static inline void app_u32_to_str(uint32_t value, char* out) {
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

static inline void app_byte_to_hex(unsigned char value, char* out) {
    static const char hex[] = "0123456789ABCDEF";
    out[0] = hex[(value >> 4) & 0x0F];
    out[1] = hex[value & 0x0F];
    out[2] = '\0';
}

static inline void app_copy_args(char* dst, int max_len) {
    int i = 0;
    while (ARG_BUFFER[i] != '\0' && i < max_len - 1) {
        dst[i] = ARG_BUFFER[i];
        i++;
    }
    dst[i] = '\0';
}

static inline void app_split_args(char* input, char** first, char** second) {
    char* p = input;
    while (*p == ' ') p++;
    *first = p;

    while (*p != '\0' && *p != ' ') p++;
    if (*p == '\0') {
        *second = p;
        return;
    }

    *p++ = '\0';
    while (*p == ' ') p++;
    *second = p;
}

#endif
