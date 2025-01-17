#ifndef FS_H
#define FS_H

#include "../libc/libc.h"

typedef struct {
    uint32_t cluster;
    uint32_t size;
    uint32_t position;
} FileHandle;

/**
 * Initializes the file system
 * @return 0 on success, non-zero on error
 */
int fs_init();

/**
 * Opens a file by path.
 * @param path The path of the file to open.
 * @return A valid FileHandle or NULL on error.
 */
FileHandle* open_file(const char* path);

/**
 * Reads data from an open file
 * @param handle The file handle
 * @param buffer The buffer to read data into
 * @param size The number of bytes to read
 * @return The number of bytes read, or -1 on error.
 */
int read_file(FileHandle* handle, void* buffer, size_t size);

/**
 * Closes a file
 * @param handle The file handle to close
 */
void close_file(FileHandle* handle);