#include <stdint.h>
#include <idt.h>
#include <disk.h>

#define VGA_ADDRESS 0xB8000
extern DirectoryEntry root_directory[16];

void syscall_handler(struct regs *r) {
    uint32_t syscall_num = r->eax;

    if (syscall_num == 1) { /* sys_write(str, line, col) */
        char* str = (char*)r->ebx;
        int line = r->ecx;
        int col = r->edx;
        uint16_t* vga = (uint16_t*) VGA_ADDRESS;
        int index = (line * 80) + col;
        for (int i = 0; str[i] != '\0' && index < 2000; i++) {
            vga[index++] = (uint16_t) str[i] | (uint16_t) 0x0F << 8;
        }
    } 
    else if (syscall_num == 2) { /* sys_read() -> char */
        volatile char* key_ptr = (volatile char*)0x9000;
        r->eax = (uint32_t)*key_ptr;
        if (*key_ptr != 0) *key_ptr = 0;
    }
    else if (syscall_num == 3) { /* sys_clear() */
        uint16_t* vga = (uint16_t*) VGA_ADDRESS;
        for (int i = 0; i < 2000; i++) vga[i] = (uint16_t) ' ' | (uint16_t) 0x00 << 8;
    }
    else if (syscall_num == 4) { /* sys_ls(buffer) */
        DirectoryEntry* user_dir = (DirectoryEntry*)r->ebx;
        for(int i=0; i<16; i++) {
            user_dir[i] = root_directory[i];
        }
    }
    else if (syscall_num == 5) { /* sys_read_file(name, buffer) -> success */
        char* name = (char*)r->ebx;
        char* buffer = (char*)r->ecx;
        r->eax = 0; /* Falha por padrão */
        
        extern int strcmp(const char*, const char*);
        for(int i=0; i<16; i++) {
            if(root_directory[i].active && strcmp(root_directory[i].name, name) == 0) {
                read_sectors_ATA_PIO((uint32_t)buffer, root_directory[i].start_lba, root_directory[i].sector_count);
                r->eax = 1; /* Sucesso */
                break;
            }
        }
    }
}
