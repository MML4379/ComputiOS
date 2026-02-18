#ifndef CPOS_GDT_H
#define CPOS_GDT_H

#include "types.h"

// GDT segment selectors
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE 0x18
#define GDT_USER_DATA 0x20
#define GDT_TSS 0x28

typedef struct PACKED {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} GDTEntry;

typedef struct PACKED {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_mid2;
    uint32_t base_high;
    uint32_t reserved;
} TSSDescriptor;

typedef struct PACKED {
    uint16_t limit;
    uint64_t base;
} GDTPointer;

typedef struct PACKED {
    uint32_t reserved0;
    uint64_t rsp[3]; // RSP0, RSP1, RSP2
    uint64_t reserved1;
    uint64_t ist[7]; // IST1..IST7
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} TSS;

void gdt_init(void);
void tss_set_kernel_stack(uint64_t rsp0);

#endif 
