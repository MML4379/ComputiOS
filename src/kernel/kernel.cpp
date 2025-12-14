#include "libk/types.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "pci.hpp"
#include "libk/kprint.hpp"
#include "memory_manager.hpp"
#include "vmm.hpp"
#include "heap.hpp"
#include "vga.hpp"
#include "kdm.hpp"
#include "io.hpp"
#include "../drivers/ps2/ps2_register.hpp"
#include "../drivers/ps2/ps2.hpp"
#include "graphics.hpp"

#define FB_INFO_PHYS 0x0008F000

extern "C" {
    void ps2_register();
    void ps2_kb_register();
    void ps2_mouse_register();
}

static void print_sdec(int row, int col, int64 v, uint8 attr) {
    if (v < 0) {
        print_str(row, col, "-", attr);
        print_dec(row, col + 1, (uint64)(-v), attr);
    } else {
        print_dec(row, col, (uint64)v, attr);
    }
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
    idt_init();
    serial_init();
    kputs("CPOSKRNL: Getting Ready...");
    kputs("CPOSKRNL: IDT initialized.");
    kputs("CPOSKRNL: Logging to Serial Port COM1.");
    
    pmm::init();
    vmm::init();
    heap_init();
    kputs("CPOSKRNL: Memory Management System initialized.");

    kdm::kdm_init();
    kputs("CPOSKRNL: KDM initialized.");

    // register platform drivers first (like the PS/2 drivers)
    ps2_register();
    ps2_kb_register();
    ps2_mouse_register();
    kputs("CPOSKRNL: 'PS/2 Controller Driver' registered.");
    kputs("CPOSKRNL: 'PS/2 Keyboard Driver' registered.");
    kputs("CPOSKRNL: 'PS/2 Mouse Driver' registered.");

    // enumerate the devices
    pci_init();
    kputs("CPOSKRNL: PCI intiailized.");

    kdm::Device d{};

    d = {};
    d.name = "ps2";
    d.bus  = kdm::BusType::PLATFORM;
    kdm::kdm_register_device(d);

    d = {};
    d.name = "ps2-kb";
    d.bus  = kdm::BusType::PLATFORM;
    kdm::kdm_register_device(d);

    d = {};
    d.name = "ps2-mouse";
    d.bus  = kdm::BusType::PLATFORM;
    kdm::kdm_register_device(d);

    // bind devices to drivers
    kdm::kdm_bind_all();
    kputs("CPOSKRNL: KDM device/driver bind complete.");

    pic_remap();
    outb(0x21, 0xF8);
    outb(0xA1, 0xEF);

    if (!gfx::init((gfx::FramebufferInfo*)&bootinfo->fb)) {
        kputs("CPOS_GFX: Initialization failed!");
        while (1) asm volatile("hlt");
    }

    gfx::ui_draw();
    gfx::present();

    asm volatile("sti");

    kputs("CPOSKRNL: Kernel OK!");

    for (;;) asm volatile("hlt");
}
