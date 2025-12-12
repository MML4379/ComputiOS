#include "idt.hpp"
#include "libk/types.hpp"
#include "pic.hpp"
#include "io.hpp"
#include "vga.hpp"

extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();

// IRQ 0–15 -> vectors 32–47 after PIC remap
extern "C" void isr32();
extern "C" void isr33();
extern "C" void isr34();
extern "C" void isr35();
extern "C" void isr36();
extern "C" void isr37();
extern "C" void isr38();
extern "C" void isr39();
extern "C" void isr40();
extern "C" void isr41();
extern "C" void isr42();
extern "C" void isr43();
extern "C" void isr44();
extern "C" void isr45();
extern "C" void isr46();
extern "C" void isr47();

extern "C" void idt_flush(uint64 idt_descriptor_ptr);
extern "C" void exception_handler(uint64 vec, uint64 rip);
extern "C" void irq_handler(uint64 vec);

static IdtEntry idt[256];
static IdtDescriptor idt_desc;

// must match cs defined in gdt
static const uint16 KERNEL_CS = 0x18;

static void set_idt_gate(int n, void (*handler)()) {
    uint64 addr = (uint64)handler;
    idt[n].offset_low  = (uint16)(addr & 0xFFFF);
    idt[n].selector    = KERNEL_CS;
    idt[n].ist         = 0;
    idt[n].type_attr   = 0x8E;      // present, ring0, interrupt gate
    idt[n].offset_mid  = (uint16)((addr >> 16) & 0xFFFF);
    idt[n].offset_high = (uint32)((addr >> 32) & 0xFFFFFFFF);
    idt[n].zero        = 0;
}

void idt_init() {
    // Zero the IDT
    for (int i = 0; i < 256; ++i) {
        idt[i].offset_low  = 0;
        idt[i].selector    = 0;
        idt[i].ist         = 0;
        idt[i].type_attr   = 0;
        idt[i].offset_mid  = 0;
        idt[i].offset_high = 0;
        idt[i].zero        = 0;
    }

    // Exceptions 0–31
    set_idt_gate(0,  isr0);
    set_idt_gate(1,  isr1);
    set_idt_gate(2,  isr2);
    set_idt_gate(3,  isr3);
    set_idt_gate(4,  isr4);
    set_idt_gate(5,  isr5);
    set_idt_gate(6,  isr6);
    set_idt_gate(7,  isr7);
    set_idt_gate(8,  isr8);
    set_idt_gate(9,  isr9);
    set_idt_gate(10, isr10);
    set_idt_gate(11, isr11);
    set_idt_gate(12, isr12);
    set_idt_gate(13, isr13);
    set_idt_gate(14, isr14);
    set_idt_gate(15, isr15);
    set_idt_gate(16, isr16);
    set_idt_gate(17, isr17);
    set_idt_gate(18, isr18);
    set_idt_gate(19, isr19);
    set_idt_gate(20, isr20);
    set_idt_gate(21, isr21);
    set_idt_gate(22, isr22);
    set_idt_gate(23, isr23);
    set_idt_gate(24, isr24);
    set_idt_gate(25, isr25);
    set_idt_gate(26, isr26);
    set_idt_gate(27, isr27);
    set_idt_gate(28, isr28);
    set_idt_gate(29, isr29);
    set_idt_gate(30, isr30);
    set_idt_gate(31, isr31);

    // IRQs 0–15 mapped to IDT 32–47
    set_idt_gate(32, isr32);
    set_idt_gate(33, isr33);
    set_idt_gate(34, isr34);
    set_idt_gate(35, isr35);
    set_idt_gate(36, isr36);
    set_idt_gate(37, isr37);
    set_idt_gate(38, isr38);
    set_idt_gate(39, isr39);
    set_idt_gate(40, isr40);
    set_idt_gate(41, isr41);
    set_idt_gate(42, isr42);
    set_idt_gate(43, isr43);
    set_idt_gate(44, isr44);
    set_idt_gate(45, isr45);
    set_idt_gate(46, isr46);
    set_idt_gate(47, isr47);

    idt_desc.limit = (uint16)(sizeof(idt) - 1);
    idt_desc.base  = (uint64)&idt[0];

    idt_flush((uint64)&idt_desc);
}

// exception handler
extern "C" void exception_handler(uint64 vec, uint64 rip) {
    // Red header
    for (int col = 0; col < 80; ++col) {
        putc_at(0, col, ' ', 0x4F); // white on red background
    }

    print_str(0, 2, "CPU EXCEPTION: ", 0x4F);
    print_dec(0, 17, vec, 0x4F);

    // we get RIP from RSI (saved by the ISR stub), now get CS/RFLAGS from the
    // stack. We can't rely on RSP position because the compiler creates a
    // prologue, so just read the stack with an explicit asm that uses RSP.
    uint64 cs = 0;
    uint64 rflags = 0;
    __asm__ __volatile__("mov 16(%%rsp), %%rax\n\t" /* saved CS */
                         "mov 24(%%rsp), %%rbx\n\t" /* saved RFLAGS */
                         : "=a"(cs), "=b"(rflags) : : "rcx");

    print_str(1, 2, "Saved RIP: 0x", 0x4F);
    print_hex(1, 16, rip, 0x4F);
    print_str(2, 2, "Saved CS: 0x", 0x4F);
    print_hex(2, 12, cs, 0x4F);
    print_str(3, 2, "Saved RFLAGS: 0x", 0x4F);
    print_hex(3, 19, rflags, 0x4F);

    // Attempt to read up to 8 bytes of the instruction at RIP and print them
    uint64 maybe_instr = 0;
    if (rip) {
        // Attempt to read 8 bytes at 'rip'; use volatile to avoid compiler
        // reordering. This can fault, but hopefully the handler can detect
        // and print something sensible.
        maybe_instr = *((volatile uint64*)rip);
        print_str(4, 2, "Instr bytes: 0x", 0x4F);
        print_hex(4, 16, maybe_instr, 0x4F);
    }

    if (vec == 14) {  // Page Fault
        uint64 cr2;
        __asm__ __volatile__("mov %%cr2, %0" : "=r"(cr2));
        print_str(1, 2, "Page fault at: 0x", 0x4F);
        print_hex(1, 20, cr2, 0x4F);
        print_str(2, 2, "System halted.", 0x4F);
    } else {
        print_str(1, 2, "System halted.", 0x4F);
    }

    while (1) {
        __asm__ __volatile__("hlt");
    }
}


// irq handler

static uint64 g_ticks = 0;
static uint8  g_last_scancode = 0;

extern "C" void irq_handler(uint64 vec) {
    uint8 irq = (uint8)(vec - 32);  // 32–47 -> 0–15

    if (irq == 0) {
        // Timer tick
        ++g_ticks;
        // Show ticks in green on row 3
        print_str(3, 0, "Ticks: ", 0x0A);
        print_dec(3, 7, g_ticks, 0x0A);
    } else if (irq == 1) {
        // Keyboard
        uint8 sc = inb(0x60);
        g_last_scancode = sc;
        print_str(4, 0, "Last scancode: 0x", 0x0B);
        print_hex(4, 17, g_last_scancode, 0x0B);
    }

    pic_send_eoi(irq);
}
