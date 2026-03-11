#include <stdint.h>
#include <idt.h>
#include <disk.h>

#define VGA_ADDRESS 0xB8000
#define DIRECTORY_LBA 100

DirectoryEntry root_directory[16];

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char to_hex(uint8_t n) {
    if (n < 10) return '0' + n;
    return 'A' + (n - 10);
}

void kprint(const char* data, uint8_t color, int line, int col) {
    uint16_t* vga = (uint16_t*) VGA_ADDRESS;
    int index = (line * 80) + col;
    for (int i = 0; data[i] != '\0'; i++) {
        vga[index++] = (uint16_t) data[i] | (uint16_t) color << 8;
    }
}

void kernel_main(void) {
    init_idt();
    
    uint16_t* vga = (uint16_t*) VGA_ADDRESS;
    for (int i = 0; i < 2000; i++) vga[i] = (uint16_t) ' ' | (uint16_t) 0x1F00;

    kprint("--- COLAPSO OS V4.1 [ATA-FIX] ---", 0x0F, 0, 24);

    /* 1. Reset Buffer */
    *(volatile char*)0x9000 = 0;

    /* 2. Lendo Diretorio */
    kprint("Status: Lendo LBA 100...", 0x0E, 2, 0);
    read_sectors_ATA_PIO((uint32_t)root_directory, DIRECTORY_LBA, 1);

    int bash_idx = -1;
    for(int i=0; i<16; i++) {
        if(root_directory[i].active && strcmp(root_directory[i].name, "bash.bin") == 0) {
            bash_idx = i;
            break;
        }
    }

    if(bash_idx != -1) {
        uint32_t start_lba = root_directory[bash_idx].start_lba;
        uint32_t bash_ram = 0x20000;
        
        kprint("Status: Carregando Bash do LBA: ", 0x07, 3, 0);
        vga[3 * 80 + 31] = (uint16_t) to_hex(start_lba & 0x0F) | (uint16_t) 0x0A << 8;

        read_sectors_ATA_PIO(bash_ram, start_lba, 10);
        
        uint8_t* check = (uint8_t*)bash_ram;
        kprint("Check (0x20000): ", 0x0F, 5, 0);
        for(int i=0; i<4; i++) {
            vga[5 * 80 + 18 + (i*3)] = (uint16_t) to_hex(check[i] >> 4) | (uint16_t) 0x0B << 8;
            vga[5 * 80 + 19 + (i*3)] = (uint16_t) to_hex(check[i] & 0x0F) | (uint16_t) 0x0B << 8;
        }

        if(check[0] == 0xFA || check[0] == 0xBC || check[0] == 0xEB) {
            kprint("STATUS: Binario OK. Saltando!", 0x0A, 7, 0);
            for(volatile int i=0; i<10000000; i++);
            
            __asm__ __volatile__ ("cli");
            void (*entry)() = (void (*)())bash_ram;
            entry();
        } else {
            kprint("ERRO: Opcodes ainda apontam para MBR!", 0x0C, 7, 0);
        }
    } else {
        kprint("ERRO: Bash nao encontrado!", 0x0C, 2, 0);
    }

    while(1) {
        static uint32_t c = 0;
        if(c++ % 1000000 == 0) vga[79] ^= 0x0F00;
    }
}
