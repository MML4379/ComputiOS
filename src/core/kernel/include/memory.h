#include <cstdint>
#include <cstddef>

extern "C" void* allocate_memory(size_t size) {
    // Implement your memory allocation logic here
    // For simplicity, we'll use a basic bump allocator
    static uint8_t memory_pool[1024 * 1024]; // 1 MB memory pool
    static size_t used_memory = 0;

    if (used_memory + size <= sizeof(memory_pool)) {
        void* ptr = &memory_pool[used_memory];
        used_memory += size;
        return ptr;
    }

    // Out of memory
    return nullptr;
}

/**
extern "C" void deallocate_memory(void* ptr) {
    // Implement your memory deallocation logic here
    // For simplicity, our bump allocator doesn't support deallocation
}*/