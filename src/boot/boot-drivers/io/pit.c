// I/O operations
#define inb(port) ({ \
    unsigned char _v; \
    asm volatile ("inb %w1 , %0" : "=a" (_v) : "d" (port)); \
    _v; \
});

#define outb(port, data) \
    asm volatile ("outb %0, %w1" :: "a" (data), "d" (port))

// PIT ports
#define PIT_COMMAND_PORT 0x43
#define PIT_DATA_PORT 0x40

#include "pit.h"

void pit_init() {
    // Initialize PIT for 100Hz interrupt
    outb(PIT_COMMAND_PORT, 0b00110110);

    // Set the divisor (100)
    outb(PIT_DATA_PORT, 0x9C); // 11932 = 1193181Hz / 100Hz - 1
    outb(PIT_DATA_PORT, 0x2E);
}