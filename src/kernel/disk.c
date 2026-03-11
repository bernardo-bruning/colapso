#include <stdint.h>
#include <disk.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ( "inw %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/* Espera com timeout simples */
static void ATA_wait_BSY() {
    uint32_t timeout = 1000000;
    while((inb(0x1F7) & 0x80) && timeout--);
}

static void ATA_wait_DRQ() {
    uint32_t timeout = 1000000;
    while(!(inb(0x1F7) & 0x08) && timeout--);
}

void read_sectors_ATA_PIO(uint32_t target_address, uint32_t LBA, uint8_t sector_count) {
    ATA_wait_BSY();

    outb(0x1F6, 0xE0 | ((LBA >> 24) & 0x0F));
    outb(0x1F2, sector_count);
    outb(0x1F3, (uint8_t) LBA);
    outb(0x1F4, (uint8_t)(LBA >> 8));
    outb(0x1F5, (uint8_t)(LBA >> 16));
    outb(0x1F7, 0x20); /* Comando Read */

    uint16_t *target = (uint16_t*) target_address;

    for (int j = 0; j < sector_count; j++) {
        ATA_wait_BSY();
        ATA_wait_DRQ();

        for(int i = 0; i < 256; i++) {
            target[i] = inw(0x1F0);
        }
        target += 256;
    }
}
