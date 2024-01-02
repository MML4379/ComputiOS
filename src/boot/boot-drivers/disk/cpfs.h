#ifndef CPFS_H
#define CPFS_H

#include <stdint.h>
#include <stddef.h>

#define FILENAME_MAX_LEN 96  // Maximum length of a filename

// Structure representing a file entry
typedef struct {
    char filename[FILENAME_MAX_LEN];  // File name
    uint32_t startingCluster;         // Starting cluster of the file data
    uint32_t fileSize;                // Size of the file in bytes
} FileEntry;

// Structure representing a directory entry
typedef struct {
    FileEntry entries[32];  // Assuming a directory can hold up to 32 files
} DirectoryEntry;

// Function to initialize the file system
void initializeCPFS();

// Function to create a file in the file system
int createFile(const char* filename);

// Function to read a file from the file system
int readFile(const char* filename, void* buffer, size_t bufferSize);

#endif // CPFS_H