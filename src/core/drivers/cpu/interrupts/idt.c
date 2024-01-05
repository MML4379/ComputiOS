#include "idt.h"

// Load IDT
extern void load_idt(idt_descriptor_t* itd_desc);
static uint8_t lastScanCode = 0;

// Set IDT gate
static void set_idt_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
}

// Initialize IDT
void initIdt() {
    idt_descriptor.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idt_descriptor.base = (uint32_t)&idt;

    set_idt_gate(0, (uint32_t)kybdPress, 0x08, 0x8E);

    // Load IDT
    load_idt(&idt_descriptor);
}

void kybdPress() {
    lastScanCode = inb(0x60);
}

void cli() {
    __asm__ __volatile__("cli");
}

void sti() {
    __asm__ __volatile__("sti");
}