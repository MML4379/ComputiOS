#ifndef CPOS_IDT_H
#define CPOS_IDT_H

#include "types.h"

#define IDT_ENTRIES 256

// gate types
#define IDT_GATE_INT  0x8E // Present, DPL=0, 64-bit interrupt gate
#define IDT_GATE_TRAP 0x8F // present, DPL=0, 64-bit trap gate
#define IDT_GATE_USER 0xEE // present, DPL=3, 64-bit interrupt gate (syscalls)

typedef struct PACKED {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} IDTEntry;

typedef struct PACKED {
    uint16_t limit;
    uint64_t base;
} IDTPointer;

// interrupt frame pushed by CPU and stubs 
typedef struct PACKED {
    // pushed by stubs
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    // pushed by CPU
    uint64_t rip, cs, rflags, rsp, ss;
} InterruptFrame;

typedef void (*IRQHandler)(InterruptFrame *frame);

void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t handler, uint8_t type);
void irq_register(uint8_t irq, IRQHandler handler);

void pic_init(void);
void pic_eoi(uint8_t irq);
void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);

#endif