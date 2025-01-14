#include "fs.h"
#include "../libc/libc.h"

static DiskInfo disk_info;

int fs_init() {
    if (disk_init(&disk_info, 0x400000) != DISK_SUCCESS) {
        return -1;
    }
    return 0;
}

FileHandle* open_file(const char* path) {
    // TODO: Traverse the FAT32 directory to locate the file requested
    // For now, we can just assume a fixed cluster and size.
    FileHandle* handle = malloc(sizeof(FileHandle));
    if (!handle) return NULL;

    handle->cluster = 2; // Root dir cluster
    handle->size = 4096; // Placeholder size
    handle->position = 0;

    return handle; 
}

int read_file(FileHandle* handle, void* buffer, size_t size) {
    if (!handle || !buffer) return -1;

    uint32_t sector = handle->cluster + (handle->position / 512);
    uint32_t sector_offset = handle->position % 512;

    // Read data sector-by-sector
    char temp_buffer[512];
    size_t bytes_read = 0;
    while (bytes_read < size) {
        if (disk_read_sector(&disk_info, sector, temp_buffer, 1) != DISK_SUCCESS) {
            return -1;
        }

        size_t to_copy = 512 - sector_offset;
        if (to_copy > (size - bytes_read)) {
            to_copy = size - bytes_read;
        }

        memcpy((char*)buffer + bytes_read, temp_buffer + sector_offset, to_copy);
        bytes_read += to_copy;
        sector++;
        sector_offset = 0;
    }

    handle->position += bytes_read;
    return bytes_read;
}

void close_file(FileHandle* handle) {
    if (handle) {
        free(handle);
    }
}