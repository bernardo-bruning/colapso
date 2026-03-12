#include <stdint.h>
#include <idt.h>
#include <disk.h>

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define COM1_PORT 0x3F8
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define KLOG_SIZE 4096
#define DATA_LBA_START 500

extern DirectoryEntry root_directory[DIRECTORY_ENTRY_COUNT];
extern volatile char keyboard_buffer[];
extern volatile uint32_t keyboard_buffer_head;
extern volatile uint32_t keyboard_buffer_tail;
static int cursor_row;
static int cursor_col;
static char kernel_log[KLOG_SIZE];
static int kernel_log_len;
static char file_write_buffer[4096];

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_write(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        outb(COM1_PORT, (uint8_t)str[i]);
        if (kernel_log_len < KLOG_SIZE - 1) {
            kernel_log[kernel_log_len++] = str[i];
            kernel_log[kernel_log_len] = '\0';
        }
    }
}

static void terminal_sync_cursor(void) {
    uint16_t pos = (uint16_t)(cursor_row * VGA_WIDTH + cursor_col);
    outb(VGA_CRTC_INDEX, 0x0F);
    outb(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));
    outb(VGA_CRTC_INDEX, 0x0E);
    outb(VGA_CRTC_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

static void terminal_clear(void) {
    uint16_t* vga = (uint16_t*)VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = (uint16_t)' ' | (uint16_t)0x0F << 8;
    }
    cursor_row = 0;
    cursor_col = 0;
    terminal_sync_cursor();
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
    terminal_sync_cursor();
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
        terminal_sync_cursor();
        return;
    }

    vga[cursor_row * VGA_WIDTH + cursor_col] = (uint16_t)c | (uint16_t)0x0F << 8;
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        terminal_newline();
    } else {
        terminal_sync_cursor();
    }
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

static int strcmp_exact(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static void copy_name(char* dst, const char* src) {
    if (src[0] == '/') src++;
    int i = 0;
    for (; src[i] != '\0' && i < 15; i++) dst[i] = src[i];
    for (; i < 16; i++) dst[i] = '\0';
}

static int str_len(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

static int find_entry(const char* name) {
    if (name[0] == '/') name++;
    for (int i = 0; i < DIRECTORY_ENTRY_COUNT; i++) {
        if (root_directory[i].active && strcmp_casefold(root_directory[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_free_entry(void) {
    for (int i = 0; i < DIRECTORY_ENTRY_COUNT; i++) {
        if (!root_directory[i].active) return i;
    }
    return -1;
}

static uint32_t next_free_lba(void) {
    uint32_t lba = DATA_LBA_START;
    for (int i = 0; i < DIRECTORY_ENTRY_COUNT; i++) {
        if (root_directory[i].active && root_directory[i].sector_count > 0) {
            uint32_t end = root_directory[i].start_lba + root_directory[i].sector_count;
            if (end > lba) lba = end;
        }
    }
    return lba;
}

static void sync_directory(void) {
    write_sectors_ATA_PIO((uint32_t)root_directory, 100, DIRECTORY_SECTOR_COUNT);
}

void syscall_handler(struct regs *r) {
    uint32_t syscall_num = r->eax;

    if (syscall_num == 1) { /* sys_stdout_write */
        char* str = (char*)r->ebx;
        terminal_write(str);
        serial_write(str);
    } 
    else if (syscall_num == 2) { /* sys_stdin_read */
        if (keyboard_buffer_head == keyboard_buffer_tail) {
            r->eax = 0;
        } else {
            r->eax = (uint32_t)keyboard_buffer[keyboard_buffer_tail];
            keyboard_buffer_tail = (keyboard_buffer_tail + 1) % 256;
        }
    }
    else if (syscall_num == 3) { /* sys_clear */
        terminal_clear();
    }
    else if (syscall_num == 4) { /* sys_ls */
        uint8_t* src = (uint8_t*)root_directory;
        uint8_t* dest = (uint8_t*)r->ebx;
        for(uint32_t i=0; i<sizeof(DirectoryEntry) * DIRECTORY_ENTRY_COUNT; i++) { dest[i] = src[i]; }
    }
    else if (syscall_num == 5) { /* sys_read_file(name, buffer) */
        char* name = (char*)r->ebx;
        char* buffer = (char*)r->ecx;
        if (name[0] == '/') name++;
        for(int i=0; i<DIRECTORY_ENTRY_COUNT; i++) {
            if(root_directory[i].active && strcmp_casefold(root_directory[i].name, name) == 0) {
                if (root_directory[i].sector_count == 0) {
                    buffer[0] = '\0';
                    r->eax = 1;
                    return;
                }
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
        for(int i=0; i<DIRECTORY_ENTRY_COUNT; i++) {
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
    else if (syscall_num == 8) { /* sys_read_klog(buffer) */
        char* buffer = (char*)r->ebx;
        for (int i = 0; i <= kernel_log_len; i++) {
            buffer[i] = kernel_log[i];
        }
        r->eax = 1;
    }
    else if (syscall_num == 9) { /* sys_reboot() */
        outb(0x64, 0xFE);
        for (;;) {
            __asm__ __volatile__ ("cli; hlt");
        }
    }
    else if (syscall_num == 10) { /* sys_shutdown() */
        outw(0x604, 0x2000);
        for (;;) {
            __asm__ __volatile__ ("cli; hlt");
        }
    }
    else if (syscall_num == 11) { /* sys_create_file(name) */
        char* name = (char*)r->ebx;
        int idx = find_entry(name);
        if (idx == -1) idx = find_free_entry();
        if (idx == -1) {
            r->eax = 0;
            return;
        }

        copy_name(root_directory[idx].name, name);
        root_directory[idx].start_lba = 0;
        root_directory[idx].sector_count = 0;
        root_directory[idx].is_executable = 0;
        root_directory[idx].active = 1;
        sync_directory();
        r->eax = 1;
    }
    else if (syscall_num == 12) { /* sys_write_file(name, content) */
        char* name = (char*)r->ebx;
        char* content = (char*)r->ecx;
        int idx = find_entry(name);
        int content_len = str_len(content);
        uint32_t sectors = (uint32_t)((content_len + 1 + 511) / 512);

        if (sectors == 0) sectors = 1;
        if ((sectors * 512) > sizeof(file_write_buffer)) {
            r->eax = 0;
            return;
        }

        if (idx == -1) idx = find_free_entry();
        if (idx == -1) {
            r->eax = 0;
            return;
        }

        for (uint32_t i = 0; i < sectors * 512; i++) file_write_buffer[i] = 0;
        for (int i = 0; i < content_len; i++) file_write_buffer[i] = content[i];

        if (root_directory[idx].sector_count < sectors || root_directory[idx].start_lba == 0) {
            root_directory[idx].start_lba = next_free_lba();
        }

        copy_name(root_directory[idx].name, name);
        root_directory[idx].sector_count = sectors;
        root_directory[idx].is_executable = 0;
        root_directory[idx].active = 1;

        write_sectors_ATA_PIO((uint32_t)file_write_buffer, root_directory[idx].start_lba, (uint8_t)sectors);
        sync_directory();
        r->eax = 1;
    }
    else if (syscall_num == 13) { /* sys_create_dir(name) */
        char* name = (char*)r->ebx;
        int idx = find_entry(name);
        if (idx == -1) idx = find_free_entry();
        if (idx == -1) {
            r->eax = 0;
            return;
        }

        copy_name(root_directory[idx].name, name);
        root_directory[idx].start_lba = 0;
        root_directory[idx].sector_count = 0;
        root_directory[idx].is_executable = 2;
        root_directory[idx].active = 1;
        sync_directory();
        r->eax = 1;
    }
}
