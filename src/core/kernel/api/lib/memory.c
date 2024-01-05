#include "memory.h"

// Define the size of your heap
#define HEAP_SIZE 512 * (1024 * 1024)

// A basic memory block structure
typedef struct Block {
    size_t size;
    struct Block* next;
} Block;

// Define the start of the heap
static char heap[HEAP_SIZE];

// Initialize the heap with a single block
static Block* freeList = NULL;

// Function to initialize the memory allocator
void initMalloc() {
    freeList = (Block*)heap;
    freeList->size = HEAP_SIZE - sizeof(Block);
    freeList->next = NULL;
}

// Function to allocate memory
void* malloc(size_t size) {
    if (size == 0 || size > HEAP_SIZE) {
        return NULL; // Invalid size
    }

    // Align the size to the size of the block
    size = (size + sizeof(Block) - 1) / sizeof(Block) * sizeof(Block);

    Block* current = freeList;
    Block* prev = NULL;

    while (current != NULL) {
        if (current->size >= size) {
            // Allocate from this block
            if (current->size > size + sizeof(Block)) {
                // Split the block if there is enough space
                Block* newBlock = (Block*)((char*)current + sizeof(Block) + size);
                newBlock->size = current->size - size - sizeof(Block);
                newBlock->next = current->next;

                current->size = size;
                current->next = newBlock;
            }

            // Remove the allocated block from the free list
            if (prev == NULL) {
                freeList = current->next;
            } else {
                prev->next = current->next;
            }

            return (void*)((char*)current + sizeof(Block));
        }

        prev = current;
        current = current->next;
    }

    return NULL; // Out of memory
}

// Function to free memory
void free(void* ptr) {
    if (ptr == NULL) {
        return; // Invalid pointer
    }

    // Move back to the block header
    Block* block = (Block*)((char*)ptr - sizeof(Block));

    // Add the block back to the free list
    block->next = freeList;
    freeList = block;
}