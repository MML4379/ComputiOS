#include <libk/types.hpp>
#include <libk/kprint.hpp>

extern "C" int64 sys_write(uint64 fd, uint64 buf, uint64 len, uint64, uint64) {
    const char* str = (const char*)buf;
    
    for (uint64 i = 0; i < len; i++) {
        kputc(str[i]);
    }

    return len;
}