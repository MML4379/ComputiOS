#include "memory_manager.hpp"
#include <kernel/libk/kprint.hpp>
#include <kernel/libk/memory.hpp>

// Only managing memory >= 16MiB for now to avoid overlapping the kernel and boot structures, because I am yet to implement linker symbols
static const uint64 PMM_MANAGED_BASE = 16ull * 1024 * 1024;
static const uint64 FRAME_SIZE = 4096;

// We will only manage 4GiB to 16MiB for now
// 4GiB/4KiB=1048576 frames
// Bitmap needs 1048576 frames = 128KiB
static const uint64 PMM_MAX_FRAMES = 1024ull * 1024;

static uint8 pmm_bitmap[PMM_MAX_FRAMES / 8];
static uint64 pmm_frames = 0; // how many frames we actually use
static uint64 pmm_free = 0; // number of free frames
static uint64 pmm_total_mem = 0; // total usable memory (bytes)

// bitmap helpers
static inline void bitmap_set(uint64 frame) {
    pmm_bitmap[frame / 8] |=  (1u << (frame % 8));
}

static inline void bitmap_clear(uint64 frame) {
    pmm_bitmap[frame / 8] &= ~(1u << (frame % 8));
}

static inline bool bitmap_test(uint64 frame) {
    return (pmm_bitmap[frame / 8] & (1u << (frame % 8))) != 0;
}

// frame index <-> physical address (within managed region)
static inline uint64 frame_to_phys(uint64 frame_index) {
    return PMM_MANAGED_BASE + frame_index * FRAME_SIZE;
}

static inline uint64 phys_to_frame(uint64 phys) {
    return (phys - PMM_MANAGED_BASE) / FRAME_SIZE;
}

// E820 reader
static E820Entry* get_e820_entries(uint32& count) {
    uint32* count_ptr = (uint32*)0x00090000;
    count = *count_ptr;

    // entries start at 0x00090010
    E820Entry* entries = (E820Entry*)0x00090010;
    return entries;
}

namespace pmm {
    void init() {
        // mark all frames as used initially
        memset(pmm_bitmap, 0xFF, sizeof(pmm_bitmap));
        pmm_frames = 0;
        pmm_free = 0;
        pmm_total_mem = 0;

        // Read E820 entries
        uint32 e820_count = 0;
        E820Entry* e820_entries = get_e820_entries(e820_count);

        kprintf("CPOS_PMM: E820 entries = %u\n", e820_count);

        for (uint32 i = 0; i < e820_count; ++i) {
            const E820Entry& e = e820_entries[i];
            kprintf("  E820[%u]: base=0x%x length=0x%x type=%u\n",
                    i, e.base, e.length, e.type);
        }

        // find the highest usable address and total usable memory
        uint64 max_usable_end = 0;

        for (uint32 i = 0; i < e820_count; ++i) {
            const E820Entry& e = e820_entries[i];
            if (e.type != 1) continue; // not usable RAM

            uint64 region_end = e.base + e.length;
            if (region_end > max_usable_end) {
                max_usable_end = region_end;
            }

            pmm_total_mem += e.length;
        }

        // compute how many frames it'll actually manage (>= PMM_MANAGED_BASE)
        if (max_usable_end <= PMM_MANAGED_BASE) {
            // if this is true there's a problem
            kprintf("CPOS_PMM: ERROR! Could not load Physical Memory Map!\n");
            return;
        }

        uint64 managed_bytes = max_usable_end - PMM_MANAGED_BASE;
        uint64 frames_needed = managed_bytes / FRAME_SIZE;

        if (frames_needed > PMM_MAX_FRAMES) {
            frames_needed = PMM_MAX_FRAMES;
        }

        pmm_frames = frames_needed;

        // now clear bits for usable RAM regions above PMM_MANAGED_BASE
        for (uint32 i = 0; i < e820_count; ++i) {
            const E820Entry& e = e820_entries[i];
            if (e.type != 1) continue; // only "usable" memory

            uint64 region_start = e.base;
            uint64 region_end = e.base + e.length;

            // clip to managed range
            if (region_end <= PMM_MANAGED_BASE) continue; // entirely below managed region
            if (region_start < PMM_MANAGED_BASE) region_start = PMM_MANAGED_BASE;
            if (region_start >= region_end) continue;

            // convert to frame indices
            uint64 first_frame = (region_start - PMM_MANAGED_BASE) / FRAME_SIZE;
            uint64 last_frame = (region_end - PMM_MANAGED_BASE) / FRAME_SIZE;

            if (first_frame >= pmm_frames) continue;
            if (last_frame > pmm_frames) last_frame = pmm_frames;

            for (uint64 f = first_frame; f < last_frame; ++f) {
                if (bitmap_test(f)) {
                    bitmap_clear(f);
                    ++pmm_free;
                }
            }
        }

        kprintf("CPOS_PMM: managed frames=%u free=%u (~%u MiB)\n",
                pmm_frames, pmm_free, (pmm_free * FRAME_SIZE) / (1024*1024));
    }

    // allocate 1 frame (4KiB), return physical address or 0 if fails
    uint64 alloc_frame() {
        for (uint64 f = 0; f < pmm_frames; ++f) {
            if (!bitmap_test(f)) {
                // free frame
                bitmap_set(f);
                if (pmm_free > 0) --pmm_free;
                uint64 phys = frame_to_phys(f);
                return phys;
            }
        }
        return 0; // out of memory
    }

    void free_frame(uint64 addr) {
        if (addr < PMM_MANAGED_BASE) return;
        if ((addr % FRAME_SIZE) != 0) return; // must be page aligned

        uint64 f = phys_to_frame(addr);
        if (f >= pmm_frames) return;

        if (bitmap_test(f)) {
            bitmap_clear(f);
            ++pmm_free;
        }
    }

    uint64 total_memory() { return pmm_total_mem; }
    uint64 total_frames() { return pmm_frames; }
    uint64 free_frames() { return pmm_free; }
} // namespace pmm