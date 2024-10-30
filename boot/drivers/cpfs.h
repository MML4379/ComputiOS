// Define basic file system structures
#define BLOCK_SIZE 4096          // Block size of 4KB
#define JOURNAL_SIZE 64          // Size of the journal in blocks
#define CPFS_MAGIC 0x43504653      // Magic number
#define SECTOR_SIZE 512

#include "ahci.h"

// Journal entry structure
typedef struct {
    unsigned int sequence_num;   // Sequence number for order
    unsigned int block_num;      // Block number to write
    unsigned int data[BLOCK_SIZE / 4]; // Data to be written
} journal_entry_t;

// Superblock structure
typedef struct {
    unsigned int cpfs_magic;     // Magic number
    unsigned int block_count;    // Number of blocks in the filesystem
    unsigned int journal_start;  // Starting block of journal
    unsigned int journal_length; // Length of journal in blocks
} superblock_t;

// Global journal buffer
journal_entry_t journal_buffer[JOURNAL_SIZE];

int journal_write(HBA_PORT *port, unsigned int block_num, unsigned int *data);
int commit_journal(HBA_PORT *port);
bool check_journal(unsigned int startl, unsigned int starth, unsigned int count, unsigned int *buf);