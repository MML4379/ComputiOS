#include <cstdint>
#include <cstddef>
#include "include/memory.h"
#include "include/screen.h"

extern "C" void* allocate_memory(size_t size);

void kernel_entry_point() {
    // Set up a basic environment
    init_screen();
    print_string("Hello, kernel!\n");

    // Example: Allocate and deallocate memory
    print_string("Allocating and deallocating memory...\n");

    // Allocate memory
    uint32_t* int_ptr = static_cast<uint32_t*>(allocate_memory(sizeof(uint32_t)));
    if (int_ptr) {
        *int_ptr = 42;
        print_string("Allocated integer: ");
        print_dec(*int_ptr);
        print_string("\n");
    } else {
        print_string("Memory allocation failed.\n");
    }

    // Your kernel initialization code goes here

    // Infinite loop
    while (1) {
        // Your kernel code goes here
    }
}