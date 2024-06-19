// ComputiOS Loader

#include <stdint.h>
#include "vga/vga.h"
#include "disk/ata.h"

#define KERNEL_LOAD_ADDRESS 0x100000
#define KERNEL_START_SECTOR 0x100
#define KERNEL_NUM_SECTORS  128

// Prototype for the kernel entry function
extern void kernel_entry(void* kernel_address);

int load_kernel(void* address);

void loader_main() {
    // Initialize VGA
    vga_init();

    // Load kernel into memory
    void* kernel_address = (void*)KERNEL_LOAD_ADDRESS;
    if (!load_kernel(kernel_address)) {
        vga_write_string("Error: Failed to load kernel.\nSystem halted.");
        while (1) {}
    }

    // Transfer control to the kernel
    kernel_entry(kernel_address);

    // If we reach this point, there was an error
    vga_write_string("Error: Kernel returned control to bootloader.\nSystem halted.");
    while (1) {}
}

int load_kernel(void* address) {
    // Load the kernel
    ata_read_sectors(KERNEL_START_SECTOR, KERNEL_NUM_SECTORS, address);

    // Assuming the load was successful, return 1
    return 1;
}