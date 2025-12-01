#include "heap.hpp"
#include "memory_manager.hpp"
#include "vmm.hpp"
#include "libk/kprint.hpp"
#include "libk/memory.hpp"

static const uint64 HEAP_START = 0xFFFF800000100000ULL;
static const uint64 HEAP_LIMIT = 0xFFFF800010000000ULL; // 256 MiB of virtual heap
static const uint64 PAGE_SIZE = 4096;
static const uint64 ALIGNMENT = 16;

struct BlockHeader {
    size_t size; // size of user payload
    bool free;
    BlockHeader* next;
    BlockHeader* prev;
};

static BlockHeader* heap_head = nullptr;
static uint64 heap_curr_end = HEAP_START;

static inline uint64 align_up(uint64 addr, uint64 align) { return (addr + align - 1) & ~(align - 1); }

// map pages so that "needed_end" is mapped
static bool expand_heap(uint64 needed_end) {
    if (needed_end > HEAP_LIMIT) {
        kprintf("CPOS_HEAP: reached heap limit!\n");
        return false;
    }

    // map page-by-page from heap_curr_end up to needed_end
    while (heap_curr_end < needed_end) {
        uint64 virt_page = heap_curr_end & ~0xFFFULL;

        // allocate physical frame
        uint64 phys_frame = pmm::alloc_frame();
        if (phys_frame == 0) {
            kprintf("CPOS_HEAP: out of physical memory!\n");
            return false;
        }

        bool ok = vmm::map_page(virt_page, phys_frame, vmm::PAGE_RW);
        if (!ok) {
            kprintf("CPOS_HEAP: failed to map heap page at %x\n", virt_page);
            return false;
        }

        heap_curr_end = virt_page + PAGE_SIZE;
    }

    return true;
}

void heap_init() {
    heap_head = nullptr;
    heap_curr_end = HEAP_START;
    kprintf("CPOS_HEAP: initialized, virtual [%x, %x]\n",
            HEAP_START, HEAP_LIMIT);
}

void* kmalloc(size_t size) {
    if (size == 0) return nullptr;

    // align size
    size = align_up(size, ALIGNMENT);

    // first-fit search for a free block
    BlockHeader* curr = heap_head;
    while (curr) {
        if (curr->free && curr->size >= size) {
            // found a suitable block
            curr->free = false;
            return (void*)((uint64)curr + sizeof(BlockHeader));
        }
        curr = curr->next;
    }

    // no suitable block found; need to allocate a new one
    uint64 total_size = sizeof(BlockHeader) + size;
    uint64 needed_end = align_up((heap_head ? (uint64)heap_head + heap_head->size + sizeof(BlockHeader) : HEAP_START) + total_size, PAGE_SIZE);

    if (!expand_heap(needed_end)) {
        return nullptr; // failed to expand heap
    }

    // create new block at the end of the heap
    BlockHeader* new_block = (BlockHeader*)(heap_curr_end - total_size);
    new_block->size = size;
    new_block->free = false;
    new_block->next = nullptr;
    new_block->prev = nullptr;

    // link into the list
    if (!heap_head) {
        heap_head = new_block;
    } else {
        BlockHeader* tail = heap_head;
        while (tail->next) tail = tail->next;
        tail->next = new_block;
        new_block->prev = tail;
    }

    return (void*)((uint64)new_block + sizeof(BlockHeader));
}

void kfree(void* ptr) {
    if (!ptr) return;

    BlockHeader* block = (BlockHeader*)((uint64)ptr - sizeof(BlockHeader));
    block->free = true;

    // coalescing with next block if free
    if (block->next && block->next->free) {
        BlockHeader* next = block->next;
        block->size += sizeof(BlockHeader) + next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
    }
}