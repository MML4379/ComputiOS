/*
    ComputiOS Kernel
    Copyright (C) 2024 MML Tech LLC
    Licensed under the MIT License
*/

int main() {
    // Simple "Hello, World!" kernel
    char *hello = "Hello, World!";
    print_string("Hello", 0);
    
    // Halt the system (in a real kernel, you'd have an infinite loop or more functionality)
    while (1) {
        __asm__("hlt");
    }
}

// Helper function to print a string
void print_string(const char *str, int *success) {
    while (*str != '\0') {
        __asm__ volatile (
            "int $0x10"
            : "=a" (*success) // Output: AX register set to the success status
            : "a" (*str), "d" (0x0E) // Input: AX register set to character, DH register set to display page
        );

        // Check if the printing was successful
        if (*success != 0) {
            break;
        }

        str++;
    }
}