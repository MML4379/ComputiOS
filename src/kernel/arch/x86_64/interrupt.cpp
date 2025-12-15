#include "interrupt.hpp"
#include "pic.hpp"
#include <drivers/input/ps2.hpp>
#include <libk/kprint.hpp>
#include <panic.hpp>
#include <mm/mm.hpp>
#include <mm/vmm.hpp>
#include <mm/memory_manager.hpp>

extern "C" void exception_dispatch(InterruptFrame* frame) {
    if (frame->vector == 14) {
        auto info = decode_pf(frame->error);
        uint64 cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
        if (info.reserved) panic("#PF: reserved bit violation");
        if (info.present) panic("#PF: protection violation");
        if (!info.present &&
            cr2 >= KERNEL_HEAP_START &&
            cr2 < KERNEL_HEAP_END) {
                uint64 page = cr2 & ~0xFFF;

                uint64* phys = (uint64*)pmm::alloc_frame();
                if (!phys) panic("Out of memory in PF handler!");

                vmm::map_page(page, *phys, vmm::PAGE_PRESENT | vmm::PAGE_RW);

                return;
            }

        kputs("PAGE FAULT");
        kprintf("  addr: %x", cr2);
        kprintf("  err : %x", frame->error);
        kprintf("  rip : %x", frame->rip);

        // Do NOT halt yet
        return;
    }

    panic("Unhandled exception");
}

extern "C" void irq_dispatch(InterruptFrame* f) {
    uint8 irq = f->vector - 32;

    if (irq == 0) {
        // timer
    } else if (irq == 1 || irq == 12) {
        ps2::on_irq(irq);
    }

    pic_send_eoi(irq);
}