#include "memory.hpp"

extern "C" {
    void* memset(void* dst, int value, size_t count) {
        uint8* d = (uint8*)dst;
        uint8 v = (uint8)value;
        for (uint64 i = 0; i < count; ++i) {
            d[i] = v;
        }
        return dst;
    }

    void* memcpy(void* dst, const void* src, size_t count) {
        uint8* d = (uint8*)dst;
        const uint8* s = (const uint8*)src;
        for (uint64 i = 0; i < count; ++i) {
            d[i] = s[i];
        }
        return dst;
    }

    void* memmove(void* dst, const void* src, size_t count) {
        uint8* d = (uint8*)dst;
        const uint8* s = (const uint8*)src;

        if (d == s || count == 0) {
            return dst;
        }

        if (d < s) {
            // safe to copy forward
            for (uint64 i = 0; i < count; ++i) {
                d[i] = s[i];
            }
        } else {
            // overlapping, copy backwards
            for (uint64 i = count; i > 0; --i) {
                d[i-1] = s[i-1];
            }
        }

        return dst;
    }

    int memcmp(const void* a, const void* b, size_t count) {
        const uint8* p1 = (const uint8*)a;
        const uint8* p2 = (const uint8*)b;
        
        for (uint64 i = 0; i < count; ++i) {
            if (p1[i] < p2[i]) return -1;
            if (p1[i] > p2[i]) return 1;
        }
        return 0;
    }
} // extern "C"