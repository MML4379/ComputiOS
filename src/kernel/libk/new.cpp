#include <mm/heap.hpp>
#include <libk/types.hpp>

extern "C" void* operator new(size_t size) { return kmalloc(size); }

extern "C" void operator delete(void* ptr) noexcept { kfree(ptr); }

extern "C" void* operator new[](size_t size) { return kmalloc(size); }

extern "C" void operator delete[](void* ptr) noexcept { kfree(ptr); }