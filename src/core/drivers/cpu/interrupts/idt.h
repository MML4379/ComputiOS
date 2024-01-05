#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t offset_low; // Lower 16 bits of handler function address
    uint16_t selector;   // Code segment sector
    uint8_t zero;        // Always 0
    uint8_t type_attr;   // Type and atttributes
    uint16_t offset_high; // Higher 16 bits of handler function address
} __attribute__((packed)) idt_entry_t;

#define IDT_ENTRIES 256

idt_entry_t idt[IDT_ENTRIES];

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_descriptor_t;

idt_descriptor_t idt_descriptor;

void cli();
void sti();