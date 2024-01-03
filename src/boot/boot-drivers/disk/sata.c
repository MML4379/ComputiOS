#include "sata.h"
#include "../../krnl-loading/loadKrnl.h"
#include "../io/pit.h" 

// SATA registers (example addresses, adjust based on your hardware)
#define SATA_DATA_REG 0x1F0
#define SATA_ERROR_REG 0x1F1
#define SATA_SECTOR_COUNT_REG 0x1F2
#define SATA_SECTOR_NUMBER_REG 0x1F3
#define SATA_LBA_LOW_REG 0x1F3
#define SATA_LBA_MID_REG 0x1F4
#define SATA_LBA_HIGH_REG 0x1F5
#define SATA_DRIVE_HEAD_REG 0x1F6
#define SATA_COMMAND_REG 0x1F7
#define SATA_STATUS_REG 0x1F7

// SATA status bits
#define SATA_STATUS_ERR_BIT 0x01
#define SATA_STATUS_DRQ_BIT 0x08
#define SATA_STATUS_BUSY_BIT 0x80

// SATA commands
#define SATA_CMD_READ_SECTORS 0x20
#define SATA_CMD_WRITE_SECTORS 0x30

// AHCI register offsets
#define AHCI_PORT_CMD_LIST_BASE 0x00
#define AHCI_PORT_CMD_TABLE_BASE 0x08
#define AHCI_PORT_FIS_BASE 0x10
#define AHCI_PORT_INT_STATUS 0x18
#define AHCI_PORT_COMMAND 0x38
#define AHCI_PORT_STATUS 0x28

// AHCI command flags
#define AHCI_CMD_FIS_LENGTH_EX 0x80000000
#define AHCI_CMD_ATAPI 0x80000000
#define AHCI_CMD_WRITE 0x40000000
#define AHCI_CMD_PREFETCHABLE 0x20000000
#define AHCI_CMD_RESET 0x10000000
#define AHCI_CMD_BIST 0x01000000
#define AHCI_CMD_CLEAR_BUSY 0x02000000
#define AHCI_CMD_DMA 0x00000008
#define AHCI_CMD_READ 0x00000000

// Sector size
#define SECTOR_SIZE 0x200

// Function to wait until the SATA controller is ready
static int waitForSataReady() {
    while ((inb(SATA_STATUS_REG) & SATA_STATUS_BUSY_BIT) || (inb(SATA_STATUS_REG) & SATA_STATUS_DRQ_BIT) == 0);
    return 1;
}

// Function to initialize the disk driver
void initializeDiskDriver() {
    // Disable interrupts
    __asm__ __volatile__("cli");

    // Initialize PIT (newly added)
    pit_init();

    // Reset the SATA controller (software reset)
    outb(SATA_COMMAND_REG, 0x04);
    // Wait a short time for the reset to complete
    for (volatile int i = 0; i < 100000; ++i);

    // Check the status of the SATA controller
    if (inb(SATA_STATUS_REG) != 0xFF) {
        errorHalt(ERROR_DISK_NOT_READY, "Error: SATA controller is not ready!");
        return;
    }

    // Enable interrupts
    __asm__ __volatile__("sti");
}

// Function to read data from the disk
int readFromDisk(uint64_t sector, size_t count, void* buffer) {
    // Wait until the SATA controller is ready
    if (!waitForSataReady()) {
        errorHalt(ERROR_DISK_NOT_READY, "Error: SATA controller is not ready!");
        return -1;  // Error: SATA controller not ready
    }

    // Set up registers for read operation
    outb(SATA_SECTOR_COUNT_REG, count);
    outb(SATA_LBA_LOW_REG, (uint8_t) sector);
    outb(SATA_LBA_MID_REG, (uint8_t)(sector >> 8));
    outb(SATA_LBA_HIGH_REG, (uint8_t)(sector >> 16));
    outb(SATA_DRIVE_HEAD_REG, 0xE0 | ((sector >> 24) & 0x0F));  // Set drive and use LBA mode
    outb(SATA_COMMAND_REG, SATA_CMD_READ_SECTORS);  // Issue read command

    // Read data from the data port
    for (size_t i = 0; i < count; ++i) {
        if (!waitForSataReady()) {
            errorHalt(ERROR_DISK_NOT_READY, "Failed to read data: SATA controller not ready");
            return -2;  // Error: SATA controller not ready for data
        }

        // Read a sector into the buffer
        insl(SATA_DATA_REG, buffer + i * SECTOR_SIZE, SECTOR_SIZE / 4);
    }

    return 0;  // Return 0 for success, or an error code
}

// Function to write data to the disk
int writeToDisk(uint64_t sector, size_t count, const void* buffer) {
    // Wait until the SATA controller is ready
    if (!waitForSataReady()) {
        errorHalt(ERROR_DISK_NOT_READY, "Error: SATA controller is not ready!");
        return -1;  // Error: SATA controller not ready
    }

    // Set up registers for write operation
    outb(SATA_SECTOR_COUNT_REG, count);
    outb(SATA_LBA_LOW_REG, (uint8_t) sector);
    outb(SATA_LBA_MID_REG, (uint8_t)(sector >> 8));
    outb(SATA_LBA_HIGH_REG, (uint8_t)(sector >> 16));
    outb(SATA_DRIVE_HEAD_REG, 0xE0 | ((sector >> 24) & 0x0F));  // Set drive and use LBA mode
    outb(SATA_COMMAND_REG, SATA_CMD_WRITE_SECTORS);  // Issue write command

    // Write data to the data port
    for (size_t i = 0; i < count; ++i) {
        if (!waitForSataReady()) {
            errorHalt(ERROR_DISK_NOT_READY, "Failed to write data: SATA controller not ready");
            return -2;  // Error: SATA controller not ready for data
        }

        // Write a sector from the buffer
        outsl(SATA_DATA_REG, buffer + i * SECTOR_SIZE, SECTOR_SIZE / 4);
    }

    return 0;  // Return 0 for success, or an error code
}