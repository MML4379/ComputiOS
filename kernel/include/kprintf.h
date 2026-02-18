#ifndef CPOS_KPRINTF_H
#define CPOS_KPRINTF_H

#include <types.h>

// Variadic support - freestanding, use compiler builtins
typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)

// Kernel formatted print - outputs to serial port 1
int kprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int ksprintf(char *buf, const char *fmt, ...);
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

// Serial output (Serial Port 1 - 0x3F8) 
void serial_init(void);
void serial_putc(char c);
void serial_puts(const char *s);

#endif