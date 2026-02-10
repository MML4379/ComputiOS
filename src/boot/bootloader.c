#include "io.h"
#include "types.h"

struct FramebufferInfo {
    uint64_t physBase;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
};

void read_sectors_ata(uint32_t lba, uint8_t count, void* buffer) {
    // Select Master Drive (0xE0) + top 4 bits of LBA
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    // Null byte (mostly ignored)
    outb(0x1F1, 0x00);
    // Sector count
    outb(0x1F2, count);
    // LBA Low
    outb(0x1F3, (uint8_t)lba);
    // LBA Mid
    outb(0x1F4, (uint8_t)(lba >> 8));
    // LBA High
    outb(0x1F5, (uint8_t)(lba >> 16));
    // Command: READ SECTORS (0x20)
    outb(0x1F7, 0x20);

    // Wait for drive to be ready (Poll Status Register)
    // We should do this for every sector if count > 1
    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < count; i++) {
        // Wait for BSY to clear and DRQ to set
        while ((inb(0x1F7) & 0x88) != 0x08);

        // Read 256 words (512 bytes)
        for (int j = 0; j < 256; j++) {
            *ptr = inw(0x1F0);
            ptr++;
        }
    }
}

// Entry point called by Stage 2
// The attribute forces the function name to stay 'loader_main' for the linker
extern void loader_main(struct FramebufferInfo* fb, void* memmap, uint32_t boot_drive) {
    
    // 1. Draw a Background (Blue)
    uint32_t* screen = (uint32_t*)(uintptr_t)fb->physBase;
    for(uint32_t i = 0; i < fb->width * fb->height; i++) {
        screen[i] = 0xFF000088; // ARGB
    }

    // wait to do this
    // uint8_t* fs_buffer = (uint8_t*)0x20000;
    // read_sectors_ata(0, 1, fs_buffer);

    __asm__ volatile("hlt");
}