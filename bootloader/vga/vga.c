#include <stdint.h>
#include <stddef.h>
#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDRESS 0xB8000

static uint16_t* const VGA_BUFFER = (uint16_t*)VGA_ADDRESS;
static uint8_t terminal_row;
static uint8_t terminal_column;
static uint8_t terminal_color;

void vga_init(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear_screen();
}

void vga_clear_screen(void) {
    for (uint8_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint8_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_BUFFER[index] = vga_entry(' ', terminal_color);
        }
    }
}

void vga_write_char(char c) {
    if (c == '\n') {
        terminal_row++;
        terminal_column = 0;
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        VGA_BUFFER[index] = vga_entry(c, terminal_color);
        terminal_column++;
        if (terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }
}

void vga_write_string(const char* str) {
    while (*str) {
        vga_write_char(*str++);
    }
}

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}