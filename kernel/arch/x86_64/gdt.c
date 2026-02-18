// GDT Code

#include <gdt.h>
#include <kstring.h>

static GDTEntry gdt_entries[7] ALIGNED(16);
static GDTPointer gdt_ptr;
static TSS tss ALIGNED(16);

extern void gdt_flush(uint64_t gdt_ptr_addr);
extern void tss_flush(uint16_t selector);

static void gdt_set_entry(int idx, uint8_t access, uint8_t gran) {
    gdt_entries[idx].limit_low   = 0xFFFF;
    gdt_entries[idx].base_low    = 0;
    gdt_entries[idx].base_mid    = 0;
    gdt_entries[idx].access      = access;
    gdt_entries[idx].granularity = gran;
    gdt_entries[idx].base_high   = 0;
}

void gdt_init(void) {
    // null
    memset(&gdt_entries[0], 0, sizeof(GDTEntry));

    // kernel code
    gdt_set_entry(1, 0x9A, 0xAF);  // Present, Ring0, Code, Read, Long Mode 

    // kernel data
    gdt_set_entry(2, 0x92, 0xCF);  // Present, Ring0, Data, Write 

    // user code
    gdt_set_entry(3, 0xFA, 0xAF);  // Present, Ring3, Code, Read, Long Mode 

    // user data
    gdt_set_entry(4, 0xF2, 0xCF);  // Present, Ring3, Data, Write 

    // Entry 5-6: TSS (0x28) - 16-byte descriptor in long mode 
    memset(&tss, 0, sizeof(TSS));
    tss.iomap_base = sizeof(TSS);

    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(TSS) - 1;

    TSSDescriptor *tss_desc = (TSSDescriptor *)&gdt_entries[5];
    tss_desc->limit_low   = tss_limit & 0xFFFF;
    tss_desc->base_low    = tss_base & 0xFFFF;
    tss_desc->base_mid    = (tss_base >> 16) & 0xFF;
    tss_desc->access      = 0x89;  // Present, TSS (available, 64-bit)
    tss_desc->granularity = (tss_limit >> 16) & 0x0F;
    tss_desc->base_mid2   = (tss_base >> 24) & 0xFF;
    tss_desc->base_high   = (uint32_t)(tss_base >> 32);
    tss_desc->reserved    = 0;

    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base  = (uint64_t)&gdt_entries;

    gdt_flush((uint64_t)&gdt_ptr);
    tss_flush(GDT_TSS);
}

void tss_set_kernel_stack(uint64_t rsp0) {
    tss.rsp[0] = rsp0;
}