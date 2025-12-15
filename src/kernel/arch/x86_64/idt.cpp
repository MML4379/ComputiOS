#include "idt.hpp"
#include "pic.hpp"
#include <drivers/input/ps2.hpp>

extern "C" void idt_flush(uint64);
extern "C" void *isr_stub_table[];

extern "C" void exception_dispatch(InterruptFrame* frame);
extern "C" void irq_dispatch(InterruptFrame* frame);

static IdtEntry idt[256];
static IdtDescriptor idt_desc;

static constexpr uint16 KERNEL_CS = 0x08;

static void set_gate(int vec, void* handler, uint8 ist = 0) {
    uint64 addr = (uint64)handler;
    idt[vec] = {
        (uint16)(addr & 0xFFFF),
        KERNEL_CS,
        ist,
        0x8E,
        (uint16)((addr >> 16) & 0xFFFF),
        (uint32)(addr >> 32),
        0
    };
}

void idt_init() {
    for (int i = 0; i < 256; i++) {
        set_gate(i, isr_stub_table[i], (i == 8 || i == 14) ? 1 : 0);
    }

    idt_desc = { sizeof(idt) - 1, (uint64)&idt };
    idt_flush((uint64)&idt_desc);
}