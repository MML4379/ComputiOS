#include "scheduler.hpp"
#include "heap.hpp"
#include "thread.hpp"
#include "libk/kprint.hpp"
#include "libk/memory.hpp"

extern "C" void context_switch(ThreadContext* old_ctx, ThreadContext* new_ctx);

// simple counters
static uint32 next_thread_id = 1;

// run queues per priority
static Thread* run_queues[scheduler::NUM_PRIORITIES] = { nullptr };

// currently running thread
static Thread* current_thread = nullptr;

// for initial main thread context
static Thread main_thread;

// forward decl
extern "C" void thread_trampoline_c();

namespace scheduler {
    Thread* current() { return current_thread; }
    
    static void enqueue_thread(Thread* t) {
        if (t->state != ThreadState::Runnable) return;
        uint32 pr = t->priority;
        if (pr >= NUM_PRIORITIES) pr = NUM_PRIORITIES - 1;

        Thread* head = run_queues[pr];
        if (!head) {
            run_queues[pr] = t;
            t->next = nullptr;
        } else {
            Thread* cur = head;
            while (cur->next) cur = cur->next;
            cur->next = t;
            t->next = nullptr;
        }
    }

    static Thread* dequeue_next_runnable() {
        for (uint32 pr = 0; pr < NUM_PRIORITIES; ++pr) {
            Thread* head = run_queues[pr];
            Thread* prev = nullptr;
            Thread* cur = head;

            while (cur) {
                if (cur->state == ThreadState::Runnable) {
                    // remove from list
                    if (prev) prev->next = cur->next;
                    else run_queues[pr] = cur->next;
                    cur->next = nullptr;
                    return cur;
                }

                prev = cur;
                cur = cur->next;
            }
        }
        
        return nullptr; // no runnable thread
    }

    void init() {
        // initialize run queues
        for (uint32 i = 0; i < NUM_PRIORITIES; ++i) run_queues[i] = nullptr;

        // set up main_thread as the currently running thread
        memset(&main_thread, 0, sizeof(Thread));
        main_thread.id = next_thread_id++;
        main_thread.priority = 1; // mid priority
        main_thread.state = ThreadState::Runnable;
        main_thread.entry = nullptr;
        main_thread.arg = nullptr;
        main_thread.stack_base = nullptr;
        main_thread.stack_size = 0;

        // capture current rsp into main_thread.ctx.rsp
        uint64 rsp_val;
        __asm__ __volatile__("mov %%rsp, %0" : "=r"(rsp_val));
        main_thread.ctx.rsp = rsp_val;

        current_thread = &main_thread;
        kprintf("SCHEDULER: initialized, main thread id=%u\n", main_thread.id);
    }

    // stack layout helper
    static void setup_new_thread_stack(Thread* t) {
        const uint64 stack_top = (uint64)t->stack_base + t->stack_size;

        // the first context_switch should return into thread_trampoline_c
        uint64* stack = (uint64*)stack_top;

        // stack grows down
        stack--; // space for fake return RIP
        *stack = (uint64)thread_trampoline_c;

        t->ctx.rsp = (uint64)stack;

        // zero callee-saved registers
        t->ctx.r15 = 0;
        t->ctx.r14 = 0;
        t->ctx.r13 = 0;
        t->ctx.r12 = 0;
        t->ctx.rbx = 0;
        t->ctx.rbp = 0;
    }

    Thread* create_thread(void (*entry)(void*), void* arg, uint8 priority) {
        Thread* t = (Thread*)kmalloc(sizeof(Thread));
        if (!t) {
            kprintf("SCHEDULER: create_thread failed: out of memory\n");
            return nullptr;
        }

        memset(t, 0, sizeof(Thread));
        t->id = next_thread_id++;
        t->priority = (priority < NUM_PRIORITIES) ? priority : (NUM_PRIORITIES - 1);
        t->state = ThreadState::Runnable;
        t->entry = entry;
        t->arg = arg;

        const uint64 stack_size = 16 * 1024; // 16 KiB
        void* stack = kmalloc(stack_size);
        if (!stack) {
            kprintf("SCHEDULER: create_thread stack allocation failed!\n");
            return nullptr;
        }
        t->stack_base = stack;
        t->stack_size = stack_size;

        setup_new_thread_stack(t);

        enqueue_thread(t);
        kprintf("SCHEDULER: created thread id=%u priority=%u\n", t->id, t->priority);
        return t;
    }

    // yields the CPU to another thread of same or higher priority, if none are found yield to lower priority thread.
    __attribute__((noinline))
    void yield() {
        kprintf("SCHEDULER: yield() entered, current id=%u\n",
            current_thread ? current_thread->id : 0xFFFFFFFF);

        Thread* self = current_thread;
        if (!self) return;

        if (self->state == ThreadState::Runnable) {
            enqueue_thread(self);
        }

        Thread* next = nullptr;

        // 1) Try same or higher priority
        for (uint32 pr = 0; pr <= self->priority && pr < NUM_PRIORITIES; ++pr) {
            Thread* head = run_queues[pr];
            Thread* prev = nullptr;
            Thread* cur  = head;

            while (cur) {
                if (cur->state == ThreadState::Runnable && cur != self) {
                    if (prev) prev->next = cur->next;
                    else run_queues[pr] = cur->next;
                    cur->next = nullptr;
                    next = cur;
                    break;
                }
                prev = cur;
                cur  = cur->next;
            }
            if (next) break;
        }

        // 2) try lower priorities
        if (!next) {
            for (uint32 pr = self->priority + 1; pr < NUM_PRIORITIES; ++pr) {
                Thread* head = run_queues[pr];
                Thread* prev = nullptr;
                Thread* cur  = head;

                while (cur) {
                    if (cur->state == ThreadState::Runnable && cur != self) {
                        if (prev) prev->next = cur->next;
                        else run_queues[pr] = cur->next;
                        cur->next = nullptr;
                        next = cur;
                        break;
                    }
                    prev = cur;
                    cur  = cur->next;
                }
                if (next) break;
            }
        }

        if (!next) {
            kprintf("SCHEDULER: yield() found no other runnable threads\n");
            return;
        }

        kprintf("SCHEDULER: switching from %u to %u\n", self->id, next->id);

        Thread* prev_thread = self;
        current_thread = next;
        context_switch(&prev_thread->ctx, &next->ctx);
    }

    void start() {
        // from kernel_main: after creating threads, call scheduler::start()
        // this will pick the first runnable thread and switch to it

        Thread* next = dequeue_next_runnable();
        if (!next) {
            kprintf("SCHEDULER: no threads to schedule.\n");
            return;
        }

        Thread* prev = current_thread;
        current_thread = next;

        kprintf("SCHEDULER: starting scheduler, switching to thread id=%u\n", next->id);
        context_switch(&prev->ctx, &next->ctx);
        // when the scheduler eventually switches back to main_thread, execution resumes here (or at least it should)
    }
} // namespace scheduler

// thread_trampoline_c implementation
// called when a new thread is first scheduled
extern "C" void thread_trampoline_c() {
    Thread* t = scheduler::current();
    if (!t || !t->entry) {
        kprintf("SCHEDULER: invalid thread context in trampoline!\n");
        while (1) { __asm__ __volatile__("hlt"); }
    }

    t->entry(t->arg);

    // thread function returned
    t->state = ThreadState::Finished;
    kprintf("SCHEDULER: thread id=%u exited.\n", t->id);

    // yield to another thread; this never returns
    scheduler::yield();

    // if it somehow comes back, halt
    while (1) { __asm__ __volatile__("hlt"); }
}