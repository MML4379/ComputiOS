/**
    ComputiOS Loader
    Copyright (C) 2024 MML Tech LLC
    Licensed under the MIT license.
*/

#include "loadKrnl.h"
#include "longMode.h"
#include "../boot-drivers/vga/video.h"
#include "../boot-drivers/disk/sata.h"
#include "../boot-drivers/disk/cpfs.h"
#include "../boot-drivers/io/pit.h"
#include <malloc.h>
#include <string.h>

void checkForKernel() {
    // Initialize the CPFS (File System) driver
    initializeCPFS();

    // Try to open the kernel file
    int openResult = openFile("cposkrnl.mapp");

    // Check if the file was opened successfully
    if (openResult != 0) {
        // Show error and halt if the kernel file is not found
        errorHalt(ERROR_KERNEL_INITIALIZATION, "Error: Kernel not found (1:/cposkrnl.mapp)");
    }
}

void loadKrnl() {    
    switchToLongMode();
    // Load necessary drivers
    loadDrivers();
    checkForKernel();

    // Enable keyboard (implementation pending)

    // Load the kernel into RAM
    int kernelSize = getFileSize("cposkrnl.mapp");
    void* kernelBuffer = malloc(kernelSize);

    // Read the kernel file into the buffer
    int readResult = readFile("cposkrnl.mapp", kernelBuffer, kernelSize);
    if (readResult != 0) {
        // Show error and halt if there's an issue reading the kernel
        errorHalt(ERROR_KERNEL_INITIALIZATION, "Error: Unable to read kernel (1:/cposkrnl.mapp)");
    }

    // Define the address where the kernel will be copied
    uintptr_t kernelAddress = 0x100000;

    // Copy the kernel to the specified address
    memcpy((void*)kernelAddress, kernelBuffer, kernelSize);

    // execute your kernel from the copied address
    ((void (*)(void))kernelAddress)();

    // Free the allocated buffer
    free(kernelBuffer);
}

void loadDrivers() {
    // Initialize necessary drivers
    initializeDiskDriver();
    initializeVideoDriver();
    pit_init();
}

void errorHalt(int errCode, const char* errText) {
    // Clear the screen
    clearScreen();
    
    // Display error code
    writeString("Error code: ");
    writeInteger(errCode);
    writeString(" - Reason: ");
    writeString(errText);
}