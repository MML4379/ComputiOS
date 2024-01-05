#include <stddef.h>

#ifndef MEMORY_H
#define MEMORY_H

void initMalloc();
void *malloc(size_t size);
void free(void* ptr);

#endif // MEMORY_H