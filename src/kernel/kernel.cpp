// ComputiOS Kernel
// https://github.com/MML4379/ComputiOS

#include "kernel.hpp"
#include "panic.hpp"
#include <arch/x86_64/gdt.hpp>
#include <arch/x86_64/tss.hpp>
#include <libk/kprint.hpp>
#include <libk/types.hpp>
#include <arch/x86_64/idt.hpp>
#include <arch/x86_64/pic.hpp>
#include <arch/x86_64/serial.hpp>
#include <arch/x86_64/io.hpp>
#include <mm/memory_manager.hpp>
#include <mm/vmm.hpp>
#include <mm/heap.hpp>
#include <drivers/kdm.hpp>
#include <drivers/pci.hpp>
#include <drivers/input/ps2.hpp>
#include <drivers/input/ps2_register.hpp>
#include <gfx/graphics.hpp>

#define FB_INFO_PHYS 0x0008F000

extern "C" {
    void ps2_register();
    void ps2_kb_register();
    void ps2_mouse_register();
    void syscall_init();
    void tss_flush();
}

struct BootFramebuffer {
    uint64 phys_base;
    uint32 width;
    uint32 height;
    uint32 pitch;
    uint32 bpp;
    uint32 format;
} __attribute__((packed));

BootFramebuffer boot_fb;

struct BootInfo {
    BootFramebuffer* fb;
};

BootInfo* bootinfo = (BootInfo*)FB_INFO_PHYS;

extern "C" void kernel_main() {
    // 1. CPU state
    gdt_init();
    tss_init();
    idt_init();
    tss_flush();

    // 2. Early logging
    serial_init();
    kputs("CPOSKRNL: Getting Ready...");
    kputs("CPOSKRNL: GDT, TSS & IDT initialized.");
    kputs("CPOSKRNL: Logging to Serial Port COM1.");

    // 3. Memory
    pmm::init();
    vmm::init();
    heap_init();
    kputs("CPOSKRNL: Memory Management System initialized.");

    // 4. Interrupt routing
    pic_remap();
    outb(0x21, 0xF8);
    outb(0xA1, 0xEF);

    // 5. Syscalls
    syscall_init();
    kputs("CPOSKRNL: Syscall interface initialized.");

    // 6. Driver model
    kdm::kdm_init();
    kputs("CPOSKRNL: KDM initialized.");

    ps2_register();
    ps2_kb_register();
    ps2_mouse_register();

    pci_init();
    kputs("CPOSKRNL: PCI initialized.");

    kdm::Device d{};

    d = {}; d.name = "ps2"; d.bus = kdm::BusType::PLATFORM; kdm::kdm_register_device(d);
    d = {}; d.name = "ps2-kb"; d.bus = kdm::BusType::PLATFORM; kdm::kdm_register_device(d);
    d = {}; d.name = "ps2-mouse"; d.bus = kdm::BusType::PLATFORM; kdm::kdm_register_device(d);

    kdm::kdm_bind_all();
    kputs("CPOSKRNL: KDM device/driver bind complete.");

    // 7. Graphics
    if (!gfx::init((gfx::FramebufferInfo*)&bootinfo->fb)) {
        kputs("CPOS_GFX: Initialization failed!");
        while (1) asm volatile("hlt");
    }

    gfx::ui_draw();
    gfx::present();

    // 8. Interrupts ON
    asm volatile("sti");
    kputs("CPOSKRNL: Ready.");

    // 9. Idle
    for (;;) asm volatile("hlt");
}