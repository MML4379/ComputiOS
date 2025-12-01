#include "libk/types.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "pci.hpp"
#include "libk/kprint.hpp"
#include "memory_manager.hpp"
#include "vmm.hpp"
#include "heap.hpp"
#include "scheduler.hpp"

void worker_low(void* arg) {
    (void)arg;
    for (;;) {
        kprintf("   [LOW %u] before yield\n",
                scheduler::current() ? scheduler::current()->id : 0xFFFFFFFF);
        scheduler::yield();
        kprintf("   [LOW %u] after yield\n",
                scheduler::current() ? scheduler::current()->id : 0xFFFFFFFF);

        for (volatile int i = 0; i < 1000000; ++i) { }
    }
}

void worker_high(void* arg) {
    (void)arg;
    for (;;) {
        kprintf("[HIGH %u] before yield\n",
                scheduler::current() ? scheduler::current()->id : 0xFFFFFFFF);
        scheduler::yield();
        kprintf("[HIGH %u] after yield\n",
                scheduler::current() ? scheduler::current()->id : 0xFFFFFFFF);

        for (volatile int i = 0; i < 1000000; ++i) { }
    }
}

extern "C" void kernel_main() {
    idt_init();
    serial_init();
    kputs("CPOSKRNL: started.");
    kputs("CPOSKRNL: serial initialized.");

    pmm::init();
    vmm::init();
    heap_init();
    kputs("CPOSKRNL: Memory Management System initialized.");

    scheduler::init();

    // create some test threads with different priorities
    scheduler::create_thread(worker_high, nullptr, 1); // highest priority
    scheduler::create_thread(worker_low, nullptr, 1);  // lower priority

    pic_remap();
    pic_unmask();
    __asm__ __volatile__("sti");

    // hand control to scheduler
    scheduler::start();

    // when the scheduler returns back to main_thread, execution resumes here
    kputs("CPOSKRNL: returned to kernel_main after scheduler. System halted.");
    while (1) {
        __asm__ __volatile__("hlt");
    }
}