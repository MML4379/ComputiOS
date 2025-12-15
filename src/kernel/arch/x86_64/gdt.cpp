#include "gdt.hpp"
#include <kernel/libk/kprint.hpp>

struct GDTEntry {
    uint16 limit_low;
    uint16 base_low;
    uint8 base_mid;
    uint8 access;
    uint8 gran;
    uint8 base_high;
}__attribute__((packed));

struct GDTPtr {
    uint16 limit;
    uint64 base;
}__attribute__((packed));

struct TSS {
    uint32 reserved0;
    uint64 rsp0;
    uint64 rsp1;
    uint64 rsp2;
    uint64 reserved1;
    uint64 ist1;
    uint64 ist2;
    uint64 ist3;
    uint64 ist4;
    uint64 ist5;
    uint64 ist6;
    uint64 ist7;
    uint64 reserved2;
    uint16 reserved3;
    uint16 io_map;
}__attribute__((packed));

static GDTEntry gdt[7];
static GDTPtr gdt_ptr;
static TSS tss;

extern "C" void gdt_flush(uint64);
extern uint8 kernel_stack_top[];
extern uint8 ist_df_stack_top[];

static void set_entry(int i, uint32 base, uint32 limit, uint8 access, uint8 gran) {
    gdt[i] = {
        (uint16)(limit & 0xFFFF),
        (uint16)(base & 0xFFFF),
        (uint8)((base >> 16) & 0xFF),
        access,
        (uint8)(((limit >> 16) & 0x0F) | (gran & 0xF0)),
        (uint8)((base >> 24) & 0xFF)
    };
}

void gdt_init() {
    set_entry(0, 0, 0, 0, 0);

    set_entry(1, 0, 0, 0x9A, 0x20); // kernel code
    set_entry(2, 0, 0, 0x92, 0x20); // kernel data

    set_entry(3, 0, 0, 0xFA, 0x20); // user code
    set_entry(4, 0, 0, 0xF2, 0x20); // user data

    uint64 base  = (uint64)&tss;
    uint64 limit = sizeof(TSS) - 1;

    set_entry(5, base & 0xFFFFFFFF, limit, 0x89, 0x00);
    set_entry(6, base >> 32, 0, 0, 0);

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint64)&gdt;

    gdt_flush((uint64)&gdt_ptr);

    kputs("CPOSKRNL: GDT loaded");
}
