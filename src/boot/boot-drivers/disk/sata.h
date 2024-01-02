#ifndef DISK_DRIVER_H
#define DISK_DRIVER_H

#include <stddef.h>
#include <stdint.h>

// Function to initialize the disk driver
void initializeDiskDriver();

// Function to read data from the disk
int readFromDisk(uint64_t sector, size_t count, void* buffer);

// Function to write data to the disk
int writeToDisk(uint64_t sector, size_t count, const void* buffer);

#endif // DISK_DRIVER_H