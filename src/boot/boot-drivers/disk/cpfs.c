#include "cpfs.h"
#include "sata.h"

// Placeholder for the file system data structures
static DirectoryEntry rootDirectory;
#define SECTOR_SIZE 0x200 // Sector size (512 bytes)

// Function to initialize CPFS
void initializeCPFS() {
    // Clear the root directory entries
    for (size_t i = 0; i < sizeof(rootDirectory.entries) / sizeof(rootDirectory.entries[0]); ++i) {
        rootDirectory.entries[i].filename[0] = '\0';
        rootDirectory.entries[i].startingCluster = 0;
        rootDirectory.entries[i].fileSize = 0;
    }
}

// Helper function to find a file entry in the directory
static FileEntry* findFileEntry(const char* filename) {
    for (size_t i = 0; i < sizeof(rootDirectory.entries) / sizeof(rootDirectory.entries[0]); ++i) {
        if (strcmp(rootDirectory.entries[i].filename, filename) == 0) {
            return &rootDirectory.entries[i];
        }
    }
    return NULL; // File not found
}

// Function to create a file in the file system
int createFile(const char* filename) {
    // Check if the file already exists
    if (findFileEntry(filename) != NULL) {
        return -1;  // File already exists
    }

    // Find an empty slot in the directory
    size_t emptySlot = -1;
    for (size_t i = 0; i < sizeof(rootDirectory.entries) / sizeof(rootDirectory.entries[0]); ++i) {
        if (rootDirectory.entries[i].filename[0] == '\0') {
            emptySlot = i;
            break;
        }
    }

    // Check if there's an empty slot
    if (emptySlot == -1) {
        return -2;  // Directory is full
    }

    // Initialize the file entry
    FileEntry* newFile = &rootDirectory.entries[emptySlot];
    strncpy(newFile->filename, filename, FILENAME_MAX_LEN - 1);

    // Allocate a cluster for the file
    newFile->startingCluster = allocateCluster();
    if (newFile->startingCluster == 0) {
        // Handle the case where cluster allocation fails
        return -3;  // Failed to allocate cluster
    }

    newFile->fileSize = 0;  // Set the initial size to 0

    return 0;  // File created successfully
}

// Function to read a file from the file system
int readFile(const char* filename, void* buffer, size_t bufferSize) {
    // Find the file entry
    FileEntry* fileEntry = findFileEntry(filename);
    if (fileEntry == NULL) {
        return -1;  // File not found
    }

    // Read data from the disk using the disk driver
    int readResult = readFromDisk(fileEntry->startingSector, bufferSize / SECTOR_SIZE, buffer);

    return readResult;  // Return the result of the read operation
}

// Function to write a file to the file system
int writeFile(const char* filename, const void* data, size_t dataSize) {
    // Find the file entry
    FileEntry* fileEntry = findFileEntry(filename);
    if (fileEntry == NULL) {
        return -1;  // File not found
    }

    // Write data to the disk using the disk driver
    int writeResult = writeToDisk(fileEntry->startingSector, dataSize / SECTOR_SIZE, data);

    return writeResult;  // Return the result of the write operation
}

// Placeholder for the currently open file
static FileEntry* openFileEntry = NULL;

// Function to open a file in the file system
int openFile(const char* filename) {
    // Find the file entry
    openFileEntry = findFileEntry(filename);
    if (openFileEntry == NULL) {
        return -1;  // File not found
    }

    return 0;  // File opened successfully
}

// Function to get the size of a file in the file system
int getFileSize(const char* filename) {
    // Find the file entry
    FileEntry* fileEntry = findFileEntry(filename);
    if (fileEntry == NULL) {
        return -1;  // File not found
    }

    return fileEntry->fileSize;
}