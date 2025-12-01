#pragma once
#include "libk/types.hpp"

enum class ThreadState : uint8 {
    Runnable,
    Blocked,
    Finished
};

static const uint32 THREAD_PRIORITY_MAX = 3;

struct ThreadContext {
    uint64 r15;
    uint64 r14;
    uint64 r13;
    uint64 r12;
    uint64 rbx;
    uint64 rbp;
    uint64 rsp;
};

struct Thread {
    ThreadContext ctx;

    Thread* next;
    Thread* prev;

    uint32 id;
    uint8 priority; // 0 is highest, 3 is lowest
    ThreadState state;

    void (*entry)(void*);
    void* arg;

    void* stack_base;
    uint64 stack_size;
};