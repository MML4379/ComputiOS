#pragma once
#include <kernel/libk/types.hpp>

constexpr uint32 MSR_STAR   = 0xC0000081;
constexpr uint32 MSR_LSTAR  = 0xC0000082;
constexpr uint32 MSR_SFMASK = 0xC0000084;

uint64 rdmsr(uint32 msr) {
    uint32 lo, hi;
    asm volatile(
        "rdmsr"
        : "=a"(lo), "=d"(hi)
        : "c"(msr)
    );
    return ((uint64)hi << 32) | lo;
}

void wrmsr(uint32 msr, uint64 value) {
    uint32 lo = value & 0xFFFFFFFF;
    uint32 hi = value >> 32;
    asm volatile(
        "wrmsr"
        :
        : "c"(msr), "a"(lo), "d"(hi)
    );
}