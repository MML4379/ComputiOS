#pragma once
#include "libk/types.hpp"

void heap_init();
void* kmalloc(size_t size);
void kfree(void* ptr);