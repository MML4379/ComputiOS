#pragma once
#include <libk/types.hpp>
#include <drivers/kdm.hpp>

namespace ahci {
    struct HBARegs;
    struct HBAPort;

    struct Disk {
        HBAPort* port;
        uint64 sector_count;
        uint32 port_no;
    };

    // devnode ops
    int read(void* ctx, void* buf, size_t bytes);
    int write(void* ctx, const void* buf, size_t bytes);
}