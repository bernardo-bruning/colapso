#include <stdint.h>
#include <idt.h>
#include <disk.h>

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define COM1_PORT 0x3F8

extern DirectoryEntry root_directory[16];
static int cursor_row;
static int cursor_col;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_write(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        outb(COM1_PORT, (uint8_t)str[i]);
    }
}

static void terminal_clear(void) {
    uint16_t* vga = (uint16_t*)VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (uint16_t)' ' | (uint16_t)0x0F << 8;
    }
    cursor_row = 0;
    cursor_col = 0;
}

static void terminal_scroll(void) {
    uint16_t* vga = (uint16_t*)VGA_ADDRESS;
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga[(row - 1) * VGA_WIDTH + col] = vga[row * VGA_WIDTH + col];
        }
    }
    for (int col = 0; col < VGA_WIDTH; col++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = (uint16_t)' ' | (uint16_t)0x0F << 8;
    }
    cursor_row = VGA_HEIGHT - 1;
}

static void terminal_newline(void) {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= VGA_HEIGHT) terminal_scroll();
}

static void terminal_put_char(char c) {
    uint16_t* vga = (uint16_t*)VGA_ADDRESS;

    if (c == '\n') {
        terminal_newline();
        return;
    }

    if (c == '\r') {
        cursor_col = 0;
        return;
    }

    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = VGA_WIDTH - 1;
        } else {
            return;
        }
        vga[cursor_row * VGA_WIDTH + cursor_col] = (uint16_t)' ' | (uint16_t)0x0F << 8;
        return;
    }

    vga[cursor_row * VGA_WIDTH + cursor_col] = (uint16_t)c | (uint16_t)0x0F << 8;
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) terminal_newline();
}

static void terminal_write(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        terminal_put_char(str[i]);
    }
}

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

void syscall_handler(struct regs *r) {
    uint32_t syscall_num = r->eax;

    if (syscall_num == 1) { /* sys_stdout_write */
        char* str = (char*)r->ebx;
        terminal_write(str);
        serial_write(str);
    } 
    else if (syscall_num == 2) { /* sys_stdin_read */
        volatile char* key_ptr = (volatile char*)0x9000;
        r->eax = (uint32_t)*key_ptr;
        if (*key_ptr != 0) *key_ptr = 0;
    }
    else if (syscall_num == 3) { /* sys_clear */
        terminal_clear();
    }
    else if (syscall_num == 4) { /* sys_ls */
        uint8_t* src = (uint8_t*)root_directory;
        uint8_t* dest = (uint8_t*)r->ebx;
        for(uint32_t i=0; i<sizeof(DirectoryEntry)*16; i++) { dest[i] = src[i]; }
    }
    else if (syscall_num == 5) { /* sys_read_file(name, buffer) */
        char* name = (char*)r->ebx;
        char* buffer = (char*)r->ecx;
        for(int i=0; i<16; i++) {
            if(root_directory[i].active && strcmp_casefold(root_directory[i].name, name) == 0) {
                read_sectors_ATA_PIO((uint32_t)buffer, root_directory[i].start_lba, root_directory[i].sector_count);
                r->eax = 1;
                return;
            }
        }
        r->eax = 0;
    }
    else if (syscall_num == 6) { /* sys_exec(name) */
        char* name = (char*)r->ebx;
        serial_write("[EXEC ");
        serial_write(name);
        serial_write("]");
        extern int strcmp(const char*, const char*);
        for(int i=0; i<16; i++) {
            if(root_directory[i].active && strcmp(root_directory[i].name, name) == 0) {
                uint32_t app_addr = 0x40000;
                read_sectors_ATA_PIO(app_addr, root_directory[i].start_lba, root_directory[i].sector_count);
                r->eax = 1;
                return;
            }
        }
        serial_write("[EXEC MISS]");
        r->eax = 0;
    }
    else if (syscall_num == 7) { /* sys_exit() */
        serial_write("[APP EXIT]");
        r->eax = 0;
    }
}
