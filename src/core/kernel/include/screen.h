#include <cstdint>

// Define VGA color constants
enum vga_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15
};

// VGA text mode constants
const uint16_t VGA_WIDTH = 80;
const uint16_t VGA_HEIGHT = 25;
uint16_t* const VGA_MEMORY = reinterpret_cast<uint16_t*>(0xB8000);

uint8_t make_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

uint16_t make_vga_entry(char c, uint8_t color) {
    return static_cast<uint16_t>(c) | static_cast<uint16_t>(color) << 8;
}

void init_screen() {
    // Clear the screen
    for (int y = 0; y < VGA_HEIGHT; ++y) {
        for (int x = 0; x < VGA_WIDTH; ++x) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = make_vga_entry(' ', make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
        }
    }
}

void print_char(char c) {
    static uint16_t* buffer = VGA_MEMORY;
    static size_t x = 0, y = 0;

    switch (c) {
        case '\n':
            x = 0;
            ++y;
            break;
        default:
            const size_t index = y * VGA_WIDTH + x;
            buffer[index] = make_vga_entry(c, make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
            ++x;
            if (x >= VGA_WIDTH) {
                x = 0;
                ++y;
            }
            break;
    }

    if (y >= VGA_HEIGHT) {
        // Scroll up by copying each line to the line above it
        for (size_t i = 1; i < VGA_HEIGHT; ++i) {
            for (size_t j = 0; j < VGA_WIDTH; ++j) {
                const size_t from_index = i * VGA_WIDTH + j;
                const size_t to_index = (i - 1) * VGA_WIDTH + j;
                buffer[to_index] = buffer[from_index];
            }
        }

        // Clear the last line
        for (size_t j = 0; j < VGA_WIDTH; ++j) {
            const size_t last_line_index = (VGA_HEIGHT - 1) * VGA_WIDTH + j;
            buffer[last_line_index] = make_vga_entry(' ', make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
        }

        --y;
    }
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void print_dec(int num) {
    // Convert and print the integer
    // For simplicity, this is a basic conversion that may not cover all cases
    char buffer[20];  // Assume a reasonable maximum size
    int index = 0;

    if (num < 0) {
        print_char('-');
        num = -num;
    }

    do {
        buffer[index++] = '0' + num % 10;
        num /= 10;
    } while (num != 0);

    while (--index >= 0) {
        print_char(buffer[index]);
    }
}