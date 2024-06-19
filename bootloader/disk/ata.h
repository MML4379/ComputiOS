#ifndef ATA_H
#define ATA_H

#include <stdint.h>

#define ATA_PRIMARY_IO_BASE 0x1F0
#define ATA_PRIMARY_CONTROL_BASE 0x3F6
#define ATA_SECTOR_SIZE 512

void ata_read_sectors(uint32_t lba, uint8_t sector_count, void* buffer);

#endif // ATA_H