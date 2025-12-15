#pragma once

#include <libk/types.hpp>

using syscall_fn = int64(*)(uint64, uint64, uint64, uint64, uint64);

enum SYSCALL_NUMBER : uint64 {
    SYS_WRITE = 1,
    SYS_EXIT = 2,
    SYS_MAX
};

extern "C" int64 syscall_dispatch(uint64 num, uint64 a, uint64 b, uint64 c, uint64 d, uint64 e);