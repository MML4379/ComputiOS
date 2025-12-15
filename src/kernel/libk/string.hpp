#pragma once
#include "types.hpp"

extern "C" {
    uint64 strlen(const char* s);
    int strcmp(const char* a, const char* b);
    int strncmp(const char* a, const char* b, uint64 n);
    char* strcpy(char* dst, const char* src);
    char* strncpy(char* dst, const char* src, uint64 n);
    bool streq(const char* a, const char* b);
    char* strdup(const char* s);
}