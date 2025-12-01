#include "vmm.hpp"
#include "memory_manager.hpp"
#include "libk/memory.hpp"
#include "libk/kprint.hpp"

static const uint64 PAGE_SIZE = 4096;

// extract indices for the 4-level page table hierarchy
static inline uint64 pml4_index(uint64 addr) { return (addr >> 39) & 0x1FF; }
static inline uint64 pdpt_index(uint64 addr) { return (addr >> 30) & 0x1FF; }
static inline uint64 pd_index(uint64 addr) { return (addr >> 21) & 0x1FF; }
static inline uint64 pt_index(uint64 addr) { return (addr >> 12) & 0x1FF; }

// Convert a physical address to a virtual pointer. The bootloader sets up an
// identity-mapping for the very low region (first few MiB). During early
// initialization we rely on identity mapping for physical addresses in that
// range. Later the kernel should set up a proper higher-half direct map and
// update this helper accordingly.
static inline uint64* phys_to_virt(uint64 phys) { return (uint64*)phys; }

static inline uint64 get_cr3() {
    uint64 cr3;
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void invlpg(uint64 addr) {
    __asm__ __volatile__("invlpg (%0)" :: "r"(addr) : "memory");
}

static uint64* get_pml4() {
    uint64 cr3 = get_cr3();
    uint64 phys = cr3 & ~0xFFFULL;    // clear lower 12 bits
    return phys_to_virt(phys);
}

namespace vmm {
    void init() {
        uint64 cr3 = get_cr3();
        kprintf("CPOS_VMM: CR3=%x\n", cr3);
    }

    static uint64* ensure_table(uint64* parent, uint64 index, bool& created) {
        uint64 entry = parent[index];
        created = false;

        if (entry & PAGE_PRESENT) {
            // existing table
            uint64 phys = entry & ~0xFFFULL;
            return phys_to_virt(phys);
        }

        // need to allocate a new table
        uint64 frame = pmm::alloc_frame();
        if (frame == 0) return nullptr; // allocation failed

        // zero the new page table
        memset(phys_to_virt(frame), 0, PAGE_SIZE);

        // mark present and writable
        parent[index] = frame | PAGE_PRESENT | PAGE_RW;
        created = true;

        return phys_to_virt(frame);
    }

    bool map_page(uint64 virt, uint64 phys, uint64 flags) {
        if ((virt & 0xFFF) != 0 || (phys & 0xFFF) != 0) {
            kprintf("CPOS_VMM: map_page requires page-aligned addresses!\n");
            return false;
        }

        uint64* pml4 = get_pml4();
        
        bool created = false;
        uint64* pdpt = ensure_table(pml4, pml4_index(virt), created);
        if (!pdpt) return false;

        uint64* pd = ensure_table(pdpt, pdpt_index(virt), created);
        if (!pd) return false;

        uint64* pt = ensure_table(pd, pd_index(virt), created);
        if (!pt) return false;

        uint64 idx = pt_index(virt);

        if (pt[idx] & PAGE_PRESENT) {
            kprintf("CPOS_VMM: map_page: virtual address %x is already mapped!\n", virt);
            return false;
        }

        // Leaf entry: physical frame + flags + PRESENT
        uint64 entry_flags = flags | PAGE_PRESENT;
        pt[idx] = (phys & ~0xFFFULL) | (entry_flags & 0xFFFULL);

        invlpg(virt);
        return true;
    }

    void unmap_page(uint64 virt) {
        uint64* pml4 = get_pml4();

        uint64 l4 = pml4_index(virt);
        uint64 l3 = pdpt_index(virt);
        uint64 l2 = pd_index(virt);
        uint64 l1 = pt_index(virt);

        if (!(pml4[l4] & PAGE_PRESENT)) return;
        uint64* pdpt = phys_to_virt(pml4[l4] & ~0xFFFULL);

        if (!(pdpt[l3] & PAGE_PRESENT)) return;
        uint64* pd = phys_to_virt(pdpt[l3] & ~0xFFFULL);

        if (!(pd[l2] & PAGE_PRESENT)) return;
        uint64* pt = phys_to_virt(pd[l2] & ~0xFFFULL);

        if (!(pt[l1] & PAGE_PRESENT)) return;

        pt[l1] = 0; // clear entry
        invlpg(virt);
    }

    uint64 translate(uint64 virt) {
        uint64* pml4 = get_pml4();

        uint64 l4 = pml4_index(virt);
        uint64 l3 = pdpt_index(virt);
        uint64 l2 = pd_index(virt);
        uint64 l1 = pt_index(virt);

        if (!(pml4[l4] & PAGE_PRESENT)) return 0;
        uint64* pdpt = phys_to_virt(pml4[l4] & ~0xFFFULL);

        if (!(pdpt[l3] & PAGE_PRESENT)) return 0;
        uint64* pd = phys_to_virt(pdpt[l3] & ~0xFFFULL);

        if (!(pd[l2] & PAGE_PRESENT)) return 0;

        // handle huge pages in PD (2MiB)
        if (pd[l2] & PAGE_HUGE) {
            uint64 base = pd[l2] & ~((1ull << 21) - 1); // clear low 21 bits
            uint64 offset = virt & ((1ull << 21) - 1);
            return base + offset;
        }

        uint64* pt = phys_to_virt(pd[l2] & ~0xFFFULL);
        if (!(pt[l1] & PAGE_PRESENT)) return 0;

        uint64 base = pt[l1] & ~0xFFFULL;
        uint64 offset = virt & 0xFFF;
        return base + offset;
    }
} // namespace vmm