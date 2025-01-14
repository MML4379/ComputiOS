#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>

// Memory management
void* malloc(size_t size);
void free(void* ptr);

// String stuff
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
void* memcpy(void* dest, const void* src, size_t n);

// Integer types
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

// Boolean Types
typedef enum { false = 0, true = 1 } bool;

#endif // LIBC_H