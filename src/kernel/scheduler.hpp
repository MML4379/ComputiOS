#pragma once
#include "libk/types.hpp"
#include "thread.hpp"

namespace scheduler {
    static const uint32 NUM_PRIORITIES = 3;

    void init(); // Initialize the scheduler

    Thread* create_thread(void (*entry)(void*), void* arg, uint8 priority); // Create a new thread

    void yield(); // yield the CPU to another thread of same or higher priority
    void start(); // start the scheduler

    Thread* current(); // get currently running thread
} // namespace scheduler