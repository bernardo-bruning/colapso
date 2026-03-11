#include <stdint.h>
#include <idt.h>
#include <disk.h>

#define VGA_ADDRESS 0xB8000
#define DIRECTORY_LBA 100

DirectoryEntry root_directory[DIRECTORY_ENTRY_COUNT];

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
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

    kprint("COLAPSO KERNEL V4.2 - BASH LOADING", 15, 0, 0);

    /* 1. Lendo Diretorio do disco */
    read_sectors_ATA_PIO((uint32_t)root_directory, DIRECTORY_LBA, DIRECTORY_SECTOR_COUNT);

    /* 2. Procurando Bash no indice */
    int bash_idx = -1;
    for(int i=0; i<DIRECTORY_ENTRY_COUNT; i++) {
        if(root_directory[i].active && strcmp(root_directory[i].name, "bash.bin") == 0) {
            bash_idx = i;
            break;
        }
    }

    if(bash_idx != -1) {
        kprint("Status: Lendo Bash do Setor ", 0x07, 2, 0);
        uint32_t start_lba = root_directory[bash_idx].start_lba;
        uint32_t bash_ram = 0x20000;
        
        /* Carrega 15 setores do Bash */
        read_sectors_ATA_PIO(bash_ram, start_lba, 15);
        
        kprint("Status: Saltando para 0x20000...", 0x0A, 3, 0);
        for(volatile int i=0; i<5000000; i++);

        /* PULO FINAL */
        void (*bash_entry)() = (void (*)())0x20000;
        bash_entry();
        
    } else {
        kprint("ERRO: bash.bin nao encontrado no disco!", 0x0C, 2, 0);
    }

    while(1) {
        static uint32_t c = 0;
        if(c++ % 1000000 == 0) vga[79] ^= 0x0F00;
    }
}
