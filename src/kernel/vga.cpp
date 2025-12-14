#include "vga.hpp"
#include "libk/types.hpp"

void putc_at(int row, int col, char c, uint8 attr) {
    volatile uint16* vga = (uint16*)0xB8000;
    vga[row * 80 + col] = ((uint16)attr << 8) | (uint8)c;
}

void print_str(int row, int col, const char* s, uint8 attr) {
    while (*s) {
        putc_at(row, col, *s, attr);
        ++col;
        ++s;
    }
}

void print_dec(int row, int col, uint64 x, uint8 attr) {
    char buf[20];
    int i = 0;

    if (x == 0) {
        putc_at(row, col, '0', attr);
        return;
    }

    while (x > 0 && i < 20) {
        buf[i++] = (char)('0' + (x % 10));
        x /= 10;
    }

    while (i > 0) {
        --i;
        putc_at(row, col, buf[i], attr);
        ++col;
    }
}

void print_hex(int row, int col, uint64 val, uint8 attr) {
    for (int i = 15; i >= 0; --i) {
        uint8 nib = (val >> (i * 4)) & 0xF;
        char c = (nib < 10) ? ('0' + nib) : ('A' + (nib - 10));
        putc_at(row, col++, c, attr);
    }
}