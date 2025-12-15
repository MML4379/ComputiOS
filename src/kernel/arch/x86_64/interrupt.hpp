#pragma once
#include "idt.hpp"

struct PageFaultInfo {
    bool present;
    bool write;
    bool user;
    bool reserved;
    bool instruction;
};

static PageFaultInfo decode_pf(uint64 err) {
    return {
        .present = err & 1,
        .write = err & 2,
        .user = err & 4,
        .reserved = err & 8,
        .instruction = err & 16
    };
}

extern "C" void exception_dispatch(InterruptFrame* frame);
extern "C" void irq_dispatch(InterruptFrame* frame);