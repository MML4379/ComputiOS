#ifndef DISK_H
#define DISK_H

#include "../../apis/libc/libc.h"

// Disk codes
typedef enum {
    DISK_SUCCESS = 0,
    DISK_ERR_IO,
    DISK_ERR_NOT_FOUND,
    DISK_ERR_INVALID_CLUSTER
} Disk;

// AHCI Port Type
typedef enum {
    AHCI_PORT_UNUSED = 0,
    AHCI_PORT_SATA   = 1,
    AHCI_PORT_SATAPI = 2,
    AHCI_PORT_SEMB   = 3,
    AHCI_PORT_PM     = 4
} AhciPortType;

// Disk Information
typedef struct {
    uint32_t hba_base;     // HBA Base Address
    uint8_t* buffer;       // Temporary buffer
    uint32_t buffer_size;  // Size of temp buffer
    uint32_t port_num;     // Port index for reading disk
} DiskInfo;

// Disk Functions
Disk disk_init(DiskInfo* info, uint32_t hba_base);
Disk disk_read_sector(DiskInfo* info, uint64_t lba, void* buffer, uint32_t sector_count);

#endif // DISK_H