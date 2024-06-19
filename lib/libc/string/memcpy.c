#include <string.h>

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size) {
    unsigned char* dst = (unsigned char*) dstptr;
    unsigned char* src = (unsigned char*) srcptr;
    for(size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    return dstptr;
}