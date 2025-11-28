#pragma once
#include "types.hpp"

struct IdtEntry {
    uint16 offset_low;
    uint16 selector;
    uint8 ist;
    uint8 type_attr;
    uint16 offset_mid;
    uint32 offset_high;
    uint32 zero;
} __attribute__((packed));

struct IdtDescriptor {
    uint16 limit;
    uint64 base;
} __attribute__((packed));

void idt_init();