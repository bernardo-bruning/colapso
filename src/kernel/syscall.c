#include <stdint.h>
#include <idt.h>
#include <disk.h>

#define VGA_ADDRESS 0xB8000
#define COM1_PORT 0x3F8
extern DirectoryEntry root_directory[16];

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_write(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        outb(COM1_PORT, (uint8_t)str[i]);
    }
}

void syscall_handler(struct regs *r) {
    uint32_t syscall_num = r->eax;

    if (syscall_num == 1) { /* sys_write */
        char* str = (char*)r->ebx;
        int line = r->ecx;
        int col = r->edx;
        uint16_t* vga = (uint16_t*) VGA_ADDRESS;
        int index = (line * 80) + col;
        if (index < 0 || index >= 2000) return;
        for (int i = 0; str[i] != '\0' && index < 2000; i++) {
            vga[index++] = (uint16_t) str[i] | (uint16_t) 0x0F << 8;
        }
        serial_write(str);
    } 
    else if (syscall_num == 2) { /* sys_read */
        volatile char* key_ptr = (volatile char*)0x9000;
        r->eax = (uint32_t)*key_ptr;
        if (*key_ptr != 0) *key_ptr = 0;
    }
    else if (syscall_num == 3) { /* sys_clear */
        uint16_t* vga = (uint16_t*) VGA_ADDRESS;
        for (int i = 0; i < 2000; i++) vga[i] = (uint16_t) ' ' | (uint16_t) 0x00 << 8;
    }
    else if (syscall_num == 4) { /* sys_ls */
        uint8_t* src = (uint8_t*)root_directory;
        uint8_t* dest = (uint8_t*)r->ebx;
        for(uint32_t i=0; i<sizeof(DirectoryEntry)*16; i++) { dest[i] = src[i]; }
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
