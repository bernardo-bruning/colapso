#include <stdint.h>
#include <idt.h>
#include <disk.h>

#define VGA_ADDRESS 0xB8000
#define BUFSIZE 2000
#define SCREEN_WIDTH 80
#define DIRECTORY_LBA 100

/* Cores */
#define WHITE 15
#define LIGHT_RED 12
#define LIGHT_CYAN 11
#define BLUE 1
#define YELLOW 14
#define LIGHT_GREY 7
#define LIGHT_GREEN 10

int cursor_pos = (5 * 80) + 4; 
char cmd_buffer[80];
int cmd_idx = 0;

/* Cache do Diretorio em RAM */
DirectoryEntry root_directory[16];

/* Funções de String */
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

void kprint(const char* data, uint8_t color, int line, int col) {
    uint16_t* terminal_buffer = (uint16_t*) VGA_ADDRESS;
    int index = (line * SCREEN_WIDTH) + col;
    for (int i = 0; data[i] != '\0' && index < BUFSIZE; i++) {
        terminal_buffer[index++] = (uint16_t) data[i] | (uint16_t) color << 8;
    }
}

void newline() {
    cursor_pos = ((cursor_pos / 80) + 1) * 80 + 4;
    if (cursor_pos >= BUFSIZE - 80) {
        uint16_t* vga = (uint16_t*) VGA_ADDRESS;
        for (int i = 80 * 5; i < BUFSIZE; i++) vga[i] = (uint16_t) ' ' | (uint16_t) BLUE << 8;
        cursor_pos = (5 * 80) + 4;
    }
}

void process_command() {
    cmd_buffer[cmd_idx] = '\0';
    newline();
    
    if (strcmp(cmd_buffer, "ls") == 0) {
        kprint("Arquivos no disco:", LIGHT_CYAN, cursor_pos / 80, 4);
        for(int i=0; i<16; i++) {
            if(root_directory[i].active) {
                newline();
                kprint("- ", WHITE, cursor_pos / 80, 4);
                kprint(root_directory[i].name, root_directory[i].is_executable ? LIGHT_RED : LIGHT_GREEN, cursor_pos / 80, 6);
            }
        }
        newline();
    }
    else if (strncmp(cmd_buffer, "run ", 4) == 0) {
        char* filename = cmd_buffer + 4;
        int found = 0;
        for(int i=0; i<16; i++) {
            if(root_directory[i].active && root_directory[i].is_executable && strcmp(root_directory[i].name, filename) == 0) {
                kprint("Executando...", YELLOW, cursor_pos / 80, 4);
                uint32_t app_ram = 0x20000;
                read_sectors_ATA_PIO(app_ram, root_directory[i].start_lba, root_directory[i].sector_count);
                void (*program)() = (void (*)())app_ram;
                program(); 
                found = 1;
                newline();
                break;
            }
        }
        if(!found) kprint("Erro: Executavel nao encontrado.", LIGHT_RED, cursor_pos / 80, 4);
        newline();
    }
    else if (strncmp(cmd_buffer, "cat ", 4) == 0) {
        char* filename = cmd_buffer + 4;
        int found = 0;
        for(int i=0; i<16; i++) {
            if(root_directory[i].active && !root_directory[i].is_executable && strcmp(root_directory[i].name, filename) == 0) {
                uint32_t buffer = 0x30000;
                read_sectors_ATA_PIO(buffer, root_directory[i].start_lba, 1);
                kprint((char*)buffer, WHITE, cursor_pos / 80, 4);
                found = 1;
                newline();
                break;
            }
        }
        if(!found) kprint("Erro: Arquivo nao encontrado.", LIGHT_RED, cursor_pos / 80, 4);
        newline();
    }
    else if (strcmp(cmd_buffer, "help") == 0) {
        kprint("Comandos: ls, cat, run, clear, help", WHITE, cursor_pos / 80, 4);
        newline();
    }
    else if (strcmp(cmd_buffer, "clear") == 0) {
        uint16_t* vga = (uint16_t*) VGA_ADDRESS;
        for (int i = 80 * 5; i < BUFSIZE; i++) vga[i] = (uint16_t) ' ' | (uint16_t) BLUE << 8;
        cursor_pos = (5 * 80) + 4;
    }
    else if (cmd_idx > 0) {
        kprint("Comando desconhecido. Digite 'help'.", LIGHT_RED, cursor_pos / 80, 4);
        newline();
    }

    kprint("> ", YELLOW, cursor_pos / 80, 2);
    cmd_idx = 0;
}

void put_char(char c, uint8_t color) {
    uint16_t* terminal_buffer = (uint16_t*) VGA_ADDRESS;
    if (c == '\n') process_command();
    else if (c == '\b') {
        if (cmd_idx > 0) { cmd_idx--; cursor_pos--; terminal_buffer[cursor_pos] = (uint16_t) ' ' | (uint16_t) color << 8; }
    } 
    else if (cmd_idx < 79) {
        cmd_buffer[cmd_idx++] = c;
        terminal_buffer[cursor_pos++] = (uint16_t) c | (uint16_t) color << 8;
    }
}

void kernel_main(void) {
    init_idt();
    
    uint16_t* vga = (uint16_t*) VGA_ADDRESS;
    for (int i = 0; i < BUFSIZE; i++) vga[i] = (uint16_t) ' ' | (uint16_t) BLUE << 8;

    kprint("--- COLAPSO OS SHELL V2.2 ---", WHITE, 1, 24);
    kprint("Status: Carregando Filesystem...", YELLOW, 2, 24);

    /* CARREGAMENTO UNICO DO DIRETORIO (FORA DA IRQ) */
    read_sectors_ATA_PIO((uint32_t)root_directory, DIRECTORY_LBA, 1);

    kprint("Status: Filesystem LBA100 Pront", LIGHT_GREEN, 2, 24);
    kprint("> ", YELLOW, 5, 2); 

    __asm__ __volatile__ ("sti");

    while(1) {
        static uint32_t counter = 0;
        if (counter++ % 1000000 == 0) vga[79] ^= 0x0F00;
    }
}
