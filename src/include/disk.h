#ifndef DISK_H
#define DISK_H

#include <stdint.h>

/* Entrada de Diretório (32 bytes) */
typedef struct {
    char name[16];
    uint32_t start_lba;
    uint32_t sector_count;
    uint32_t is_executable;
    uint32_t active;
} DirectoryEntry;

void read_sectors_ATA_PIO(uint32_t target_address, uint32_t LBA, uint8_t sector_count);

#endif
