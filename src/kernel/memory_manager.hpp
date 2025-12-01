#pragma once
#include "libk/types.hpp"

struct __attribute__((packed)) E820Entry {
    uint64 base;
    uint64 length;
    uint32 type;
};

namespace pmm {
    void init(); // call this once in kernel
    uint64 alloc_frame(); // returns a physical address, 4KiB aligned, or 0 if it fails
    void free_frame(uint64 addr); // free a previously allocated frame

    uint64 total_memory(); // in bytes
    uint64 total_frames();
    uint64 free_frames();
}