#ifndef CPOS_BOOTINFO_H
#define CPOS_BOOTINFO_H

#include "types.h"

// BootInfo is passed from the bootloader via rdi
// This has to match exactly what was constructed in the bootloader

#define BOOTINFO_MAGIC 0x43504F53424F4F54ULL // "CPOSBOOT"

// E820 memory map entry
typedef struct PACKED {
    uint64_t base;
    uint64_t length;
    uint32_t type; // 1=usable, 2=Reserved, 3=ACPI reclaim, 4=ACPI NVS, 5=bad
    uint32_t acpi_ext;
} E820Entry;

#define E820_TYPE_USABLE 1
#define E820_TYPE_RESERVED 2
#define E820_TYPE_ACPI_REC 3
#define E820_TYPE_ACPI_NVS 4
#define E820_TYPE_BAD 5

typedef struct PACKED {
    uint64_t magic;

    // framebuffer
    uint64_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint32_t fb_bpp;

    // memory map
    uint64_t mmap_count;
    uint64_t mmap_addr;

    // kernel info
    uint64_t kernel_phys;
    uint64_t kernel_size;

    // ACPI
    uint64_t acpi_rsdp;
} BootInfo;

#endif