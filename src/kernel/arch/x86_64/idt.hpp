#pragma once
#include <libk/types.hpp>

struct IdtEntry {
    uint16 offset_low;
    uint16 selector;
    uint8  ist;
    uint8  type_attr;
    uint16 offset_mid;
    uint32 offset_high;
    uint32 zero;
} __attribute__((packed));

struct IdtDescriptor {
    uint16 limit;
    uint64 base;
} __attribute__((packed));

struct InterruptFrame {
    uint64 r15, r14, r13, r12, r11, r10, r9, r8;
    uint64 rdi, rsi, rbp, rbx, rdx, rcx, rax;

    uint64 vector;
    uint64 error;

    uint64 rip;
    uint64 cs;
    uint64 rflags;
    uint64 rsp;
    uint64 ss;
};

void idt_init();