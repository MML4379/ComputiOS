// IDT and PIC code

#include <idt.h>
#include <kprintf.h>
#include <kstring.h>

static IDTEntry idt_entries[IDT_ENTRIES] ALIGNED(16);
static IDTPointer idt_ptr;
static IRQHandler irq_handlers[16] = {0};

// ISR stubs
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);

// IRQ stubs 
extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

// Syscall handler
extern void isr128(void);

extern void idt_flush(uint64_t idt_ptr_addr);

// PIC (8259)
#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1
#define PIC_EOI    0x20

void pic_init(void) {
    // ICW1: begin initialization, expect ICW4
    outb(PIC1_CMD, 0x11); io_wait();
    outb(PIC2_CMD, 0x11); io_wait();

    // ICW2: remap IRQs. Master: 32-39, Slave: 40-47
    outb(PIC1_DATA, 0x20); io_wait();
    outb(PIC2_DATA, 0x28); io_wait();

    // ICW3: tell master about slave on IRQ2, tell slave its cascade identity
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();

    // ICW4: 8086 mode
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();

    // Mask all IRQs initially
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void pic_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t line = (irq < 8) ? irq : irq - 8;
    outb(port, inb(port) | (1 << line));
}

void pic_unmask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t line = (irq < 8) ? irq : irq - 8;
    outb(port, inb(port) & ~(1 << line));

    // If unmasking a slave PIC IRQ (8-15), also unmask IRQ 2 (cascade) on master
    if (irq >= 8) {
        outb(PIC1_DATA, inb(PIC1_DATA) & ~(1 << 2));
    }
}

void idt_set_gate(uint8_t num, uint64_t handler, uint8_t type) {
    idt_entries[num].offset_low = handler & 0xFFFF;
    idt_entries[num].selector = 0x08;
    idt_entries[num].ist = 0;
    idt_entries[num].type_attr = type;
    idt_entries[num].offset_mid = (handler >> 16) & 0xFFFF;
    idt_entries[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt_entries[num].reserved = 0;
}

static const char *exception_names[] = {
    "Division By Zero", "Debug", "NMI", "Breakpoint",
    "Overflow", "Bound Range", "Invalid Opcode", "Device N/A",
    "Double Fault", "Coprocessor Seg", "Invalid TSS", "Segment Not Present",
    "Stack-Segment Fault", "General Protection", "Page Fault", "Reserved",
    "x87 FP Exception", "Alignment Check", "Machine Check", "SIMD FP",
    "Virtualization", "Control Protection", "Rsvd", "Rsvd",
    "Rsvd", "Rsvd", "Rsvd", "Rsvd", "Hypervisor Inj", "VMM Comm", "Security", "Rsvd"
};

void isr_handler(InterruptFrame *frame) {
    uint64_t int_no = frame->int_no;

    if (int_no < 32) {
        // Exception
        kprintf("\n!!! EXCEPTION %llu: %s\n", int_no,
                int_no < 32 ? exception_names[int_no] : "Unknown");
        kprintf("  Error code: 0x%llx\n", frame->err_code);
        kprintf("  RIP: 0x%llx  RSP: 0x%llx\n", frame->rip, frame->rsp);
        kprintf("  CS:  0x%llx  SS:  0x%llx\n", frame->cs, frame->ss);
        kprintf("  RAX: 0x%llx  RBX: 0x%llx\n", frame->rax, frame->rbx);
        kprintf("  CR2: 0x%llx (if #PF)\n", ({
            uint64_t cr2; __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2)); cr2;
        }));

        // Halt on unrecoverable exceptions
        if (int_no == 8 || int_no == 13 || int_no == 14) {
            kprintf("  SYSTEM HALTED.\n");
            cli(); for(;;) hlt();
        }
    } else if (int_no >= 32 && int_no < 48) {
        // Hardware IRQ
        uint8_t irq = (uint8_t)(int_no - 32);
        if (irq_handlers[irq])
            irq_handlers[irq](frame);
        pic_eoi(irq);
    } else if (int_no == 128) {
        // Syscall
        extern void syscall_handler(InterruptFrame *frame);
        syscall_handler(frame);
    }
}

void irq_register(uint8_t irq, IRQHandler handler) {
    irq_handlers[irq] = handler;
    pic_unmask(irq);
}

void idt_init(void) {
    memset(idt_entries, 0, sizeof(idt_entries));

    pic_init();

    // CPU exceptions 0-31 
    void (*isrs[])(void) = {
        isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
        isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };
    for (int i = 0; i < 32; i++)
        idt_set_gate(i, (uint64_t)isrs[i], IDT_GATE_INT);

    // Hardware IRQs 0-15 -> vectors 32-47 
    void (*irqs[])(void) = {
        irq0,  irq1,  irq2,  irq3,  irq4,  irq5,  irq6,  irq7,
        irq8,  irq9,  irq10, irq11, irq12, irq13, irq14, irq15
    };
    for (int i = 0; i < 16; i++)
        idt_set_gate(32 + i, (uint64_t)irqs[i], IDT_GATE_INT);

    // Syscall gate, DPL=3 so userspace can call it
    idt_set_gate(0x80, (uint64_t)isr128, IDT_GATE_USER);

    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint64_t)&idt_entries;
    idt_flush((uint64_t)&idt_ptr);
}