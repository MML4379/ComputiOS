#include "disk.h"

#define AHCI_PORT_CMD_CR 0x8000   // Command List Running
#define AHCI_PORT_CMD_FR 0x4000   // FIS Receive Running
#define AHCI_PORT_CMD_FRE 0x0010  // FIS Receive Enable
#define AHCI_PORT_CMD_ST 0x0001   // Start
#define AHCI_PORT_CMD_CLB 0x00    // Command List Base
#define AHCI_PORT_CMD_FB  0x08    // FIS Base
#define AHCI_PORT_IS_TFES 0x40000000 // Task File Error Status

/* HBA Port Register */
typedef struct {
    uint32_t cmd_list_base;       // Command List Base Address
    uint32_t cmd_list_upper_base; // Upper 32-bits
    uint32_t fis_base;            // FIS Base Address
    uint32_t fis_upper_base;      // Upper 32-bits
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t cmd_and_status;
    uint32_t reserved[15];
} HbaPort;

/* HBA Memory Registers */
typedef struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;       // Ports Implemented
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_ports;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
    uint8_t reserved[116];
    HbaPort ports[32]; // Up to 32 ports
} HbaMem;

/* AHCI Initialization */
Disk disk_init(DiskInfo* info, uint32_t hba_base) {
    if (!info) return DISK_ERR_IO;

    HbaMem* hba = (HbaMem*)hba_base;

    // Find the first active port
    uint32_t pi = hba->pi;
    for (uint32_t i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            HbaPort* port = &hba->ports[i];
            // Check if the port is SATA
            if ((port->cmd_and_status & 0x0F) == AHCI_PORT_SATA) {
                info->hba_base = hba_base;
                info->port_num = i;
                info->buffer = malloc(4096); // Allocate a 4KB buffer
                info->buffer_size = 4096;
                return DISK_SUCCESS;
            }
        }
    }

    return DISK_ERR_NOT_INITIALIZED;
}

/* Disk Sector Read */
Disk disk_read_sector(DiskInfo* info, uint64_t lba, void* buffer, uint32_t sector_count) {
    if (!info || !buffer) return DISK_ERR_IO;

    HbaMem* hba = (HbaMem*)info->hba_base;
    HbaPort* port = &hba->ports[info->port_num];

    // Wait until the port is idle
    while (port->cmd_and_status & (AHCI_PORT_CMD_CR | AHCI_PORT_CMD_FR));

    // Stop the port
    port->cmd_and_status &= ~AHCI_PORT_CMD_ST;
    while (port->cmd_and_status & AHCI_PORT_CMD_CR);

    // Setup command and FIS buffers
    port->cmd_list_base = (uint32_t)info->buffer;
    port->fis_base = (uint32_t)info->buffer + 0x100;

    // Allocate space for the command table
    uint8_t* cmd_table_base = info->buffer + 0x200;
    memset(cmd_table_base, 0, 256); // Clear the Command Table area

    // Fill the command header
    uint8_t* cmd_header = (uint8_t*)port->cmd_list_base;
    memset(cmd_header, 0, 32); // Command Header size: 32 bytes
    cmd_header[0] = (1 << 6) | (sector_count & 0x1F); // FIS length, sector count
    cmd_header[1] = 0x00;                            // Reset, Prefetchable, Write, etc.
    cmd_header[2] = 0;                               // PRDT entries count (low)
    cmd_header[3] = 1;                               // PRDT entries count (high)

    // Configure the PRDT
    uint64_t buffer_phys = (uint64_t)buffer;         // Physical address of the buffer
    uint32_t* prdt_entry = (uint32_t*)(cmd_table_base + 0x80);
    prdt_entry[0] = (uint32_t)(buffer_phys & 0xFFFFFFFF); // Physical address (low)
    prdt_entry[1] = (uint32_t)(buffer_phys >> 32);        // Physical address (high)
    prdt_entry[2] = (sector_count * 512) - 1;            // Byte count
    prdt_entry[3] = 1;                                   // Interrupt on completion

    // Configure the FIS
    uint8_t* fis = cmd_table_base + 0x00;
    memset(fis, 0, 20); // FIS size: 20 bytes
    fis[0] = 0x27;      // Register Host-to-Device FIS
    fis[1] = 0x80;      // Command
    fis[2] = 0x25;      // READ DMA EXT
    fis[3] = (uint8_t)(lba & 0xFF);
    fis[4] = (uint8_t)((lba >> 8) & 0xFF);
    fis[5] = (uint8_t)((lba >> 16) & 0xFF);
    fis[6] = 0; // Device register
    fis[7] = (uint8_t)((lba >> 24) & 0xFF);
    fis[8] = (uint8_t)((lba >> 32) & 0xFF);
    fis[9] = (uint8_t)((lba >> 40) & 0xFF);
    fis[10] = (uint8_t)(sector_count & 0xFF); // Sector count (low byte)
    fis[11] = (uint8_t)((sector_count >> 8) & 0xFF); // Sector count (high byte)

    // Start the command
    port->cmd_and_status |= AHCI_PORT_CMD_ST;

    // Wait for completion
    while (!(port->interrupt_status & 0x01)); // Wait for the "Task Complete" bit
    if (port->interrupt_status & AHCI_PORT_IS_TFES) {
        return DISK_ERR_IO; // Handle Task File Error
    }

    // Clear interrupt status
    port->interrupt_status = port->interrupt_status;

    return DISK_SUCCESS;
}
