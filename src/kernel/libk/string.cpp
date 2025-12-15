#include "string.hpp"
#include <mm/heap.hpp>
#include <libk/memory.hpp>

extern "C" {
    uint64 strlen(const char* s) {
        uint64 len = 0;
        while (s[len] != '\0') {
            ++len;
        }
        return len;
    }

    int strcmp(const char* a, const char* b) {
        uint64 i = 0;
        while (a[i] != '\0' && b[i] != '\0') {
            if (a[i] < b[i]) return -1;
            if (a[i] > b[i]) return 1;
            ++i;
        }
        return 0;
    }

    int strncmp(const char* a, const char* b, uint64 n) {
        for (uint64 i = 0; i < n; ++i) {
            char ca = a[i];
            char cb = b[i];
            if (ca == '\0' && cb == '\0') return 0;
            if (ca < cb) return -1;
            if (ca > cb) return 1;
            if (ca == '\0' || cb == '\0') break;
        }
        return 0;
    }

    char* strcpy(char* dst, const char* src) {
        uint64 i = 0;
        while (true) {
            dst[i] = src[i];
            if (src[i] == '\0') break;
            ++i;
        }
        return dst;
    }

    char* strncpy(char* dst, const char* src, uint64 n) {
        uint64 i = 0;
        for (; i < n && src[i] != '\0'; ++i) {
            dst[i] = src[i];
        }
        for (; i < n; ++i) {
            dst[i] = '\0';
        }
        return dst;
    }

    bool streq(const char* a, const char* b) {
        if (!a || !b) return false;
        while (*a && *b) { if (*a++ != *b++) return false; }
        return *a == *b;
    }

    char* strdup(const char* s) {
        if (s == nullptr) return nullptr;

        // calculate required size
        size_t len = strlen(s);
        size_t req = len + 1;

        // allocate memory
        char* new_str = (char*)kmalloc(req);

        if (new_str == nullptr) return nullptr;

        // copy the string content
        memcpy(new_str, s, req);

        return new_str;
    }
} 