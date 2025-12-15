#include "msr.hpp"
#include <libk/types.hpp>

extern "C" void syscall_entry();

extern "C" void syscall_init() {
    // STAR:
    // bits 47:32 = kernel CS
    // bits 63:48 = user CS
    uint64 star =
        ((uint64)0x08 << 32) | // kernel CS
        ((uint64)0x18 << 48); // user CS

    wrmsr(MSR_STAR, star);
    wrmsr(MSR_LSTAR, (uint64)syscall_entry);

    // Mask interrupts on entry
    wrmsr(MSR_SFMASK, 0x200); // IF flag
}