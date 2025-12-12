#include "libk/types.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "pci.hpp"
#include "libk/kprint.hpp"
#include "memory_manager.hpp"
#include "vmm.hpp"
#include "heap.hpp"

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

    pic_remap();
    pic_unmask();
    __asm__ __volatile__("sti");

    while (1) __asm__ __volatile__("hlt");
}