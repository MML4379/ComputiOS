#include "kprint.hpp"
#include "types.hpp"
#include <stdarg.h>
#include <kernel/arch/x86_64/serial.hpp>

void kputc(char c) {
    serial_write_char(c);
}

void kputs(const char* s) {
    // print string then newline
    while (*s) {
        if (*s == '\n') {
            kputc('\r'); // CRLF for terminals
        }
        kputc(*s++);
    }
    kputc('\n');
}

// integer to string helpers

static void kprint_dec_uint64(uint64 val) {
    char buf[20];
    int i = 0;

    if (val == 0) {
        kputc('0');
        return;
    }

    while (val > 0 && i < 20) {
        uint8 digit = (uint8)(val % 10);
        buf[i++] = (char)('0' + digit);
        val /= 10;
    }

    while (i > 0) {
        --i;
        kputc(buf[i]);
    }
}

static void kprint_dec_int64(int64 val) {
    if (val < 0) {
        kputc('-');
        // careful with INT64_MIN; cast to unsigned magnitude
        uint64 mag = (uint64)(-(val + 1)) + 1;
        kprint_dec_uint64(mag);
    } else {
        kprint_dec_uint64((uint64)val);
    }
}

static void kprint_hex_uint64(uint64 val) {
    kputc('0');
    kputc('x');
    for (int i = 15; i >= 0; --i) {
        uint8 nib = (uint8)((val >> (i * 4)) & 0xF);
        char c = (nib < 10) ? ('0' + nib) : ('A' + (nib - 10));
        kputc(c);
    }
}

// --- kprintf implementation ---

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            if (*fmt == '\n') {
                kputc('\r');
            }
            kputc(*fmt++);
            continue;
        }

        // '%' encountered
        ++fmt;
        if (*fmt == '\0') break;

        char spec = *fmt++;
        switch (spec) {
        case '%':
            kputc('%');
            break;

        case 'c': {
            int c = va_arg(args, int);
            kputc((char)c);
            break;
        }

        case 's': {
            const char* s = va_arg(args, const char*);
            if (!s) s = "(null)";
            while (*s) {
                if (*s == '\n') {
                    kputc('\r');
                }
                kputc(*s++);
            }
            break;
        }

        case 'd':
        case 'i': {
            int64 v = va_arg(args, int64);
            kprint_dec_int64(v);
            break;
        }

        case 'u': {
            uint64 v = va_arg(args, uint64);
            kprint_dec_uint64(v);
            break;
        }

        case 'x':
        case 'X': {
            uint64 v = va_arg(args, uint64);
            kprint_hex_uint64(v);
            break;
        }

        default:
            // unknown specifier, print it literally
            kputc('%');
            kputc(spec);
            break;
        }
    }

    va_end(args);
}
