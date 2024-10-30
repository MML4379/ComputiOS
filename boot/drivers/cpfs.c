#include "cpfs.h"

// Write a block to the journal
int journal_write(HBA_PORT *port, unsigned int block_num, unsigned int *data) {
    static unsigned int journal_index = 0;
    
    // Check if the journal is full
    if (journal_index >= JOURNAL_SIZE) {
        printf("Journal full! Need to commit to main file system.\n");
        return -1;
    }

    // Checksum stuff
    unsigned int checksum = 0;
    for (int i = 0; i < BLOCK_SIZE / sizeof(unsigned int); i++) {
        checksum ^= data[i];
    }

    // Fill the journal entry
    journal_buffer[journal_index].sequence_num = journal_index + 1;
    journal_buffer[journal_index].block_num = block_num;
    memcpy(journal_buffer[journal_index].data, data, BLOCK_SIZE);

    // Write journal entry to disk (to the journal region)
    if (!write(port, JOURNAL_START_BLOCK + journal_index, 0, 1, (unsigned int *)&journal_buffer[journal_index])) {
        printf("Journal write failed.\n");
        return -1;
    }

    journal_index++;
    return 0;
}

// Commit the journal entries to the main file system blocks
int commit_journal(HBA_PORT *port) {
    for (int i = 0; i < JOURNAL_SIZE; i++) {
        unsigned int *data = journal_buffer[i].data;
        unsigned int block_num = journal_buffer[i].block_num;

        // Write data to the main file system block
        if (!write(port, block_num, 0, 1, data)) {
            printf("Failed to commit journal entry %d.\n", i);
            return -1;
        }
    }

    // Clear the journal buffer and reset index
    memset(journal_buffer, 0, sizeof(journal_buffer));
    journal_index = 0;

    printf("Journal commit completed successfully.\n");
    return 0;
}