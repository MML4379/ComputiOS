#include "ata.h"
#include "../io.h"
#include <stdint.h>

static inline void io_wait(void) {
    outb(0x80, 0);
}

void ata_read_sectors(uint32_t lba, uint8_t sector_count, void* buffer) {
    outb(ATA_PRIMARY_CONTROL_BASE, 0x02); // Disable IRQs

    outb(ATA_PRIMARY_IO_BASE + 0x06, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO_BASE + 1, 0x00);
    outb(ATA_PRIMARY_IO_BASE + 2, sector_count);
    outb(ATA_PRIMARY_IO_BASE + 3, (uint8_t)lba);
    outb(ATA_PRIMARY_IO_BASE + 4, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO_BASE + 5, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO_BASE + 7, 0x20);

    for (int i = 0; i < sector_count; i++) {
        while ((inb(ATA_PRIMARY_IO_BASE + 7) & 0x08) == 0);
        for (int j = 0; j < ATA_SECTOR_SIZE / 2; j++) {
            ((uint16_t*)buffer)[i * (ATA_SECTOR_SIZE / 2) + j] = inb(ATA_PRIMARY_IO_BASE);
        }
    }

    outb(ATA_PRIMARY_CONTROL_BASE, 0x00); // Enable IRQs
}