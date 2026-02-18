#ifndef CPOS_TYPES_H
#define CPOS_TYPES_H

// FREESTANDING TYPE DEFS

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
    // C++ has built-in bool, true, false. Do nothing.
#else
    // C needs these definitions 
    typedef _Bool bool;
    #define true 1
    #define false 0
#endif

#ifndef NULL
    #define NULL ((void*)0)
#endif

// Compiler attributes
#define PACKED __attribute__((packed))
#define ALIGNED(n) __attribute__((aligned(n)))
#define NORETURN __attribute__((noreturn))
#define UNUSED __attribute__((unused))
#define ALWAYS_INLINE __attribute__((always_inline)) inline

// Port I/O and CPU intristics
static ALWAYS_INLINE void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static ALWAYS_INLINE uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static ALWAYS_INLINE void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static ALWAYS_INLINE uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static ALWAYS_INLINE void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static ALWAYS_INLINE uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static ALWAYS_INLINE void io_wait(void) {
    outb(0x80, 0);  // write to an unused port for 1us delay
}

static ALWAYS_INLINE void cli(void) { __asm__ volatile ("cli"); }
static ALWAYS_INLINE void sti(void) { __asm__ volatile ("sti"); }
static ALWAYS_INLINE void hlt(void) { __asm__ volatile ("hlt"); }

static ALWAYS_INLINE uint64_t read_cr3(void) {
    uint64_t val;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

static ALWAYS_INLINE void write_cr3(uint64_t val) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(val) : "memory");
}

static ALWAYS_INLINE uint64_t rdmsr(uint32_t msr) {
    uint32_t lo, hi;
    __asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

static ALWAYS_INLINE void wrmsr(uint32_t msr, uint64_t val) {
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"((uint32_t)val), "d"((uint32_t)(val >> 32)));
}

static ALWAYS_INLINE uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

#endif // CPOS_TYPES_H