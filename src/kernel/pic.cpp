#include "pic.hpp"
#include "io.hpp"

// pic ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void pic_remap() {
    uint8 a1 = inb(PIC1_DATA);
    uint8 a2 = inb(PIC2_DATA);

    // start init
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // vector offsets: 0x20 and 0x28
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // tell Master PIC there is a slave at IRQ2
    outb(PIC1_DATA, 4);
    // tell Slave PIC its cascade identity
    outb(PIC2_DATA, 2);

    // 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // restore masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void pic_unmask() {
    // enable irq0 (timer) and irq1 (keyboard), mask the others
    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF); // all masked on slave
}

void pic_send_eoi(uint8 irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}