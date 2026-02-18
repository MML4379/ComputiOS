#include <kprintf.h>
#include <kstring.h>

#define serial_1 0x3F8

void serial_init(void) {
    outb(serial_1 + 1, 0x00); // Disable interrupts 
    outb(serial_1 + 3, 0x80); // Enable DLAB 
    outb(serial_1 + 0, 0x03); // Baud rate divisor lo (38400) 
    outb(serial_1 + 1, 0x00); // Baud rate divisor hi 
    outb(serial_1 + 3, 0x03); // 8 bits, no parity, 1 stop
    outb(serial_1 + 2, 0xC7); // Enable FIFO, clear, 14-byte threshold 
    outb(serial_1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

static int serial_transmit_ready(void) {
    return inb(serial_1 + 5) & 0x20;
}

void serial_putc(char c) {
    while (!serial_transmit_ready());
    outb(serial_1, c);
}

void serial_puts(const char *s) {
    while (*s) {
        if (*s == '\n') serial_putc('\r');
        serial_putc(*s++);
    }
}

// core output
static void kput(char c) {
    if (c == '\n') serial_putc('\r');
    serial_putc(c);
}

static void kputs(const char *s) {
    while (*s) kput(*s++);
}

// formatting
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
    size_t pos = 0;

    #define EMIT(c) do { if (pos < size - 1) buf[pos] = (c); pos++; } while(0)

    while (*fmt) {
        if (*fmt != '%') {
            EMIT(*fmt++);
            continue;
        }
        fmt++; // skip %

        // flags
        bool zero_pad = false;
        bool left_align = false;
        while (*fmt == '0' || *fmt == '-') {
            if (*fmt == '0') zero_pad = true;
            if (*fmt == '-') left_align = true;
            fmt++;
        }

        // width
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        // length modifier
        bool is_long = false;
        if (*fmt == 'l') { is_long = true; fmt++; }
        if (*fmt == 'l') { fmt++; } // ll is same as l on 64-bit

        char tmp[64];
        const char *str = tmp;
        int slen;

        switch (*fmt) {
        case 'd':
        case 'i': {
            int64_t val = is_long ? va_arg(ap, int64_t) : (int64_t)va_arg(ap, int);
            itoa(val, tmp, 10);
            break;
        }
        case 'u': {
            uint64_t val = is_long ? va_arg(ap, uint64_t) : (uint64_t)va_arg(ap, unsigned int);
            utoa(val, tmp, 10);
            break;
        }
        case 'x': {
            uint64_t val = is_long ? va_arg(ap, uint64_t) : (uint64_t)va_arg(ap, unsigned int);
            utoa(val, tmp, 16);
            break;
        }
        case 'X': {
            uint64_t val = is_long ? va_arg(ap, uint64_t) : (uint64_t)va_arg(ap, unsigned int);
            utoa(val, tmp, 16);
            // convert to uppercase
            for (int i = 0; tmp[i]; i++) { 
                if (tmp[i] >= 'a' && tmp[i] <= 'f') tmp[i] -= 32;
            }
            break;
        }
        case 'p': {
            uint64_t val = (uint64_t)va_arg(ap, void*);
            tmp[0] = '0'; tmp[1] = 'x';
            utoa(val, tmp + 2, 16);
            break;
        }
        case 's':
            str = va_arg(ap, const char*);
            if (!str) str = "(null)";
            break;
        case 'c':
            tmp[0] = (char)va_arg(ap, int);
            tmp[1] = '\0';
            break;
        case '%':
            tmp[0] = '%';
            tmp[1] = '\0';
            break;
        default:
            tmp[0] = '%';
            tmp[1] = *fmt;
            tmp[2] = '\0';
            break;
        }

        slen = (int)strlen(str);
        char pad = zero_pad ? '0' : ' ';

        if (!left_align) {
            for (int i = slen; i < width; i++) EMIT(pad);
        }
        for (int i = 0; i < slen; i++) EMIT(str[i]);
        if (left_align) {
            for (int i = slen; i < width; i++) EMIT(' ');
        }

        fmt++;
    }

    if (size > 0) buf[pos < size ? pos : size - 1] = '\0';
    return (int)pos;

    #undef EMIT
}

int ksprintf(char *buf, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = kvsnprintf(buf, 1024, fmt, ap); // caller must ensure buffer is large enough
    va_end(ap);
    return ret;
}

int kprintf(const char *fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int ret = kvsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    kputs(buf);
    return ret;
}