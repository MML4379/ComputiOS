// a basic serial driver so we can log to COM1.

#include "serial.hpp"
#include "libk/types.hpp"
#include "io.hpp"

// com1 base port
static const uint16 COM1_PORT = 0x3F8;

void serial_init() {
    outb(COM1_PORT + 1, 0x00); // disable interrupts
    outb(COM1_PORT + 3, 0x80); // set baud divisor
    outb(COM1_PORT + 0, 0x03); // divisor low byte (38400 baud)
    outb(COM1_PORT + 1, 0x00); // divisor high byte
    outb(COM1_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7); // enable FIFO, clear them, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B); // irqs enabled, rts/dsr set
}

static int serial_is_transmit_empty() {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    // wait until not busy
    while (!serial_is_transmit_empty()) {
        // spin
    }
    outb(COM1_PORT, (uint8)c);
}

void serial_write_str(const char *s) {
    while (*s) {
        if (*s == '\n') {
            serial_write_char('\r'); // CRLF for nicer terminals
        }
        serial_write_char(*s);
        ++s;
    }
}

void serial_write_hex8(uint8 v) {
    for (int i = 1; i >= 0; --i) {
        uint8 nib = (v >> (i * 4)) & 0xF;
        char c = (nib < 10)? ('0' + nib) : ('A' + (nib - 10));
        serial_write_char(c);
    }
}

void serial_write_hex16(uint16 v) {
    for (int i = 3; i >= 0; --i) {
        uint8 nib = (v >> (i * 4)) & 0xF;
        char c = (nib < 10)? ('0' + nib) : ('A' + (nib - 10));
        serial_write_char(c);
    }
}

void serial_write_hex32(uint32 v) {
    for (int i = 7; i >= 0; --i) {
        uint8 nib = (v >> (i * 4)) & 0xF;
        char c = (nib < 10)? ('0' + nib) : ('A' + (nib - 10));
        serial_write_char(c);
    }
}

void serial_write_hex64(uint64 v) {
    for (int i = 15; i >= 0; --i) {
        uint8 nib = (v >> (i * 4)) & 0xF;
        char c = (nib < 10)? ('0' + nib) : ('A' + (nib - 10));
        serial_write_char(c);
    }
}