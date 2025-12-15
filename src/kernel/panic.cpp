#include "panic.hpp"
#include <kernel/libk/kprint.hpp>

[[noreturn]] void panic(const char* msg) {
    kputs("=== KERNEL PANIC ===");
    kputs(msg);
    asm volatile("cli");
    for (;;) asm volatile("hlt");
}