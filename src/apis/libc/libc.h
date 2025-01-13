#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>

void* malloc(size_t size);
void free(void* ptr);
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
void* memcpy(void* dest, const void* src, size_t n);

#endif // LIBC_H