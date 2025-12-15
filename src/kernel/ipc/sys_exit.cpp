#include <libk/types.hpp>

extern "C" int64 sys_exit(uint64 status, uint64, uint64, uint64, uint64) {
    // kill current process/thread
    for (;;) asm volatile ("hlt");
}