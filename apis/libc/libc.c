#include "libc.h"

static char memory_pool[1024]; // A small memory pool for now
static size_t memory_index = 0;

void* malloc(size_t size) {
    if (memory_index + size > sizeof(memory_pool)) {
        return NULL; // Not enough memory
    }
    void* ptr = &memory_pool[memory_index];
    memory_index += size;
    return ptr;
}

void free(void* ptr) {
    // No-op for now (as we are not managing freed memory)
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}
