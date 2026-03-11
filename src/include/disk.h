#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define DIRECTORY_ENTRY_COUNT 64
#define DIRECTORY_SECTOR_COUNT 4

/* Entrada de Diretório (32 bytes) */
typedef struct {
    char name[16];
    uint32_t start_lba;
    uint32_t sector_count;
    uint32_t is_executable;
    uint32_t active;
} DirectoryEntry;

void read_sectors_ATA_PIO(uint32_t target_address, uint32_t LBA, uint8_t sector_count);
void write_sectors_ATA_PIO(uint32_t source_address, uint32_t LBA, uint8_t sector_count);

#endif
