#pragma once
#include "libk/types.hpp"

namespace vmm {
    // basic page flags 
    enum PageFlags : uint64 {
        PAGE_PRESENT = 1ull << 0,
        PAGE_RW = 1ull << 1,
        PAGE_USER = 1ull << 2,
        PAGE_PWT = 1ull << 3,
        PAGE_PCD = 1ull << 4,
        PAGE_ACCESSED = 1ull << 5,
        PAGE_DIRTY = 1ull << 6,
        PAGE_HUGE = 1ull << 7,
        PAGE_GLOBAL = 1ull << 8
    };

    void init();

    // map a single 4KiB page: virt -> phys with given flags (must include PAGE_RW if writable)
    // returns true on success
    bool map_page(uint64 virt, uint64 phys, uint64 flags);

    // unmaps a single page. Does NOT free the physical frame.
    void unmap_page(uint64 virt);

    // Translate a virtual address to a physical address
    // returns 0 if unmapped.
    uint64 translate(uint64 virt);
} // namespace vmm