#include "longMode.h"
#include <cstdint>

// EFER MSR register constants
#define MSR_EFER 0xC0000080
#define EFER_LME_BIT (1ULL << 8)  // Long Mode Enable bit

// Define a GDT entry structure
struct GDTEntry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMiddle;
    uint8_t access;
    uint8_t granularity;
    uint8_t baseHigh;
};

// Define a GDT descriptor structure
struct GDTDescriptor {
    uint16_t size;
    uint32_t offset;
};

// Function to set up the GDT
void setupGDT() {
    // Example GDT entries for code and data segments
    GDTEntry codeSegment = {
        .limitLow = 0xFFFF,
        .baseLow = 0x0000,
        .baseMiddle = 0x00,
        .access = 0x9A,  // Present, Ring 0, Code segment, Execute-Only
        .granularity = 0xCF,  // 4 KB granularity, 32-bit default size
        .baseHigh = 0x00
    };

    GDTEntry dataSegment = {
        .limitLow = 0xFFFF,
        .baseLow = 0x0000,
        .baseMiddle = 0x00,
        .access = 0x92,  // Present, Ring 0, Data segment
        .granularity = 0xCF,  // 4 KB granularity, 32-bit default size
        .baseHigh = 0x00
    };

    // Create a GDT descriptor
    GDTDescriptor gdtDescriptor = {
        .size = sizeof(GDTEntry) * 2 - 1,
        .offset = reinterpret_cast<std::uint32_t>(&codeSegment)
    };

    // Load the GDT
    __asm__ volatile("lgdt %0" : : "m" (gdtDescriptor));
}

// Function to switch to 64-bit long mode
void switchToLongMode() {
    // Enable long mode in EFER MSR
    std::uint64_t eferValue;

    // Read the current value of EFER MSR
    __asm__ volatile("rdmsr" : "=A" (eferValue) : "c" (MSR_EFER));

    // Set the Long Mode Enable bit
    eferValue |= EFER_LME_BIT;

    // Write the modified value back to EFER MSR
    __asm__ volatile("wrmsr" : : "c" (MSR_EFER), "A" (eferValue));

    // At this point, the CPU is in 64-bit long mode
}