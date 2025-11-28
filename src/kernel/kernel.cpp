#include "types.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "pci.hpp"

extern "C" void kernel_main() {
    idt_init(); // install the idt

    // Initialize serial
    serial_init();
    serial_write_str("CPOSKRNL: serial initialized.\n");

    // set up pic
    pic_remap();
    pic_unmask();

    // enable interrupts
    __asm__ __volatile__("sti");

    volatile uint16* vga = (uint16*)0xB8000;

    const char* msg = "ComputiOS Kernel!";
    uint32 i = 0;
    while (msg[i]) {
        vga[i] = ((uint16)0x0F << 8) | (uint8)msg[i]; // white on black
        ++i;
    }

    // test the exception handler by forcefully triggering an exception
    // __asm__ __volatile__("int $3");
    // __asm__ __volatile__("ud2");

    // Initialize PCI
    pci_init();

    while (1) {
        __asm__ __volatile__("hlt");
    }
}