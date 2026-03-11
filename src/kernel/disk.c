#include <stdint.h>
#include <disk.h>

/* Portas do Controlador ATA Primário */
#define ATA_DATA        0x1F0
#define ATA_SECCOUNT    0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_SEL   0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

/* Implementação Robusta de I/O */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* Espera o disco terminar operações pendentes */
static void ata_wait() {
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);
}

static void ata_wait_ready() {
    while (inb(ATA_STATUS) & 0x80); /* BSY bit */
    while (!(inb(ATA_STATUS) & 0x08)); /* DRQ bit */
}

void read_sectors_ATA_PIO(uint32_t target_address, uint32_t LBA, uint8_t sector_count) {
    while (inb(ATA_STATUS) & 0x80); /* Espera não estar ocupado */

    outb(ATA_DRIVE_SEL, 0xE0 | ((LBA >> 24) & 0x0F));
    outb(ATA_SECCOUNT, sector_count);
    outb(ATA_LBA_LOW,  (uint8_t)LBA);
    outb(ATA_LBA_MID,  (uint8_t)(LBA >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(LBA >> 16));
    outb(ATA_COMMAND,  0x20); /* Command: READ */

    uint16_t *buffer = (uint16_t*) target_address;

    for (int i = 0; i < sector_count; i++) {
        ata_wait_ready();
        for (int j = 0; j < 256; j++) {
            buffer[j] = inw(ATA_DATA);
        }
        buffer += 256;
    }
}

void write_sectors_ATA_PIO(uint32_t source_address, uint32_t LBA, uint8_t sector_count) {
    while (inb(ATA_STATUS) & 0x80);

    outb(ATA_DRIVE_SEL, 0xE0 | ((LBA >> 24) & 0x0F));
    outb(ATA_SECCOUNT, sector_count);
    outb(ATA_LBA_LOW,  (uint8_t)LBA);
    outb(ATA_LBA_MID,  (uint8_t)(LBA >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(LBA >> 16));
    outb(ATA_COMMAND,  0x30);

    uint16_t *buffer = (uint16_t*) source_address;

    for (int i = 0; i < sector_count; i++) {
        ata_wait_ready();
        for (int j = 0; j < 256; j++) {
            outw(ATA_DATA, buffer[j]);
        }
        ata_wait();
        buffer += 256;
    }
}
