// Is this ugly? Absolutely. Does it work? Absolutely. I do not care how it looks as long as it works, and neither should you.

#include "pci.hpp"
#include "libk/types.hpp"
#include "serial.hpp"
#include "io.hpp"

// I've probably made 8 of these already, but make it 9 since I don't feel like putting it into a separate file.
static void putc_at(int row, int col, char c, uint8 attr) {
    volatile uint16* vga = (uint16*)0xB8000;
    vga[row * 80 + col] = ((uint16)attr << 8) | (uint8)c;
}

static void print_str(int row, int col, const char* s, uint8 attr) {
    while (*s) {
        putc_at(row, col, *s, attr);
        ++col;
        ++s;
    }
}

static void print_hex32(int row, int col, uint32 val, uint8 attr) {
    for (int i = 7; i >= 0; --i) {
        uint8 nib = (val >> (i * 4)) & 0xF;
        char c = (nib < 10) ? ('0' + nib) : ('A' + (nib - 10));
        putc_at(row, col++, c, attr);
    }
}

// eventually these will be moved to the serial driver but it's a proof of concept right now
static void serial_write_hex8(uint8 v) {
    for (int i = 1; i >= 0; --i) {
        uint8 nib = (v >> (i * 4)) & 0xF;
        char c = (nib < 10)? ('0' + nib) : ('A' + (nib - 10));
        serial_write_char(c);
    }
}

static void serial_write_hex16(uint16 v) {
    for (int i = 3; i >= 0; --i) {
        uint8 nib = (v >> (i * 4)) & 0xF;
        char c = (nib < 10)? ('0' + nib) : ('A' + (nib - 10));
        serial_write_char(c);
    }
}

static void serial_write_hex32(uint32 v) {
    for (int i = 7; i >= 0; --i) {
        uint8 nib = (v >> (i * 4)) & 0xF;
        char c = (nib < 10)? ('0' + nib) : ('A' + (nib - 10));
        serial_write_char(c);
    }
}

// config addr for bus/dev/func/offset
static uint32 pci_config_address(uint8 bus, uint8 device, uint8 function, uint8 offset) {
    return (uint32)(
        (1u << 31) | // enable bit
        ((uint32)bus << 16) |
        ((uint32)device << 11) |
        ((uint32)function << 8) |
        (offset & 0xFC) // must be aligned to 4 bytes
    );
}

// read 32 bits from pci config space
static uint32 pci_config_read_32(uint8 bus, uint8 device, uint8 function, uint8 offset) {
    uint32 addr = pci_config_address(bus, device, function, offset);
    outl(0xCF8, addr);
    return inl(0xCFC);
}

void pci_init() {
    serial_write_str("PCI: Scanning buses...\n");

    int row = 6;

    for (uint8 bus = 0; bus < 32; ++bus) {
        for (uint8 dev = 0; dev < 32; ++dev) {
            for (uint8 func = 0; func < 8; ++func) {
                uint32 vendor_device = pci_config_read_32(bus, dev, func, 0x00);
                uint16 vendor = (uint16)(vendor_device & 0xFFFF);
                if (vendor == 0xFFFF) {
                    if (func == 0) break; else continue;
                }
                uint16 device = (uint16)((vendor_device >> 16) & 0xFFFF);

                // log via serial
                serial_write_str("PCI dev: bus=0x");
                serial_write_hex8(bus);
                serial_write_str(" dev=0x");
                serial_write_hex8(dev);
                serial_write_str(" func=0x");
                serial_write_hex8(func);
                serial_write_str(" vendor=0x");
                serial_write_hex16(vendor);
                serial_write_str(" device=0x");
                serial_write_hex16(device);
                serial_write_str("\n");

                // On screen: show bus/dev/func + vendor/device
                print_str(row, 0, "PCI ", 0x0E); // yellow text

                // Bus/dev/func in hex
                print_str(row, 4, "B=", 0x0E);
                print_hex32(row, 6, bus, 0x0E);

                print_str(row, 15, "D=", 0x0E);
                print_hex32(row, 17, dev, 0x0E);

                print_str(row, 26, "F=", 0x0E);
                print_hex32(row, 28, func, 0x0E);

                print_str(row, 37, "VEN=", 0x0A);
                print_hex32(row, 41, vendor, 0x0A);

                print_str(row, 50, "DEV=", 0x0A);
                print_hex32(row, 54, device, 0x0A);

                ++row;
                if (row >= 24) return; // let's not spam the entire screen
            }
        }
    }

    serial_write_str("PCI: Scan done.\n");
}