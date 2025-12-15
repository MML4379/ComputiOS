#include "syscall.hpp"
#include <libk/types.hpp>

extern "C" int64 sys_write(uint64, uint64, uint64, uint64, uint64);
extern "C" int64 sys_exit(uint64, uint64, uint64, uint64, uint64);

static syscall_fn table[SYS_MAX] = {
    nullptr,
    sys_write,
    sys_exit
};

extern "C" int64 syscall_dispatch(uint64 num, uint64 a, uint64 b, uint64 c, uint64 d, uint64 e) {
    if (num >= SYS_MAX || !table[num]) return -1;

    return table[num](a, b, c, d, e);
}