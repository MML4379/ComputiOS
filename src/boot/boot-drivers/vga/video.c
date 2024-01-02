#include "video.h"

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
volatile uint16_t* videoMemory = (uint16_t*)0xb8000; // VGA text mode buffer

void initializeVideo() {
    // Set VGA text mode
    uint16_t mode = 3;
    __asm__ __volatile__("int $0x10" : : "a"(0x4F02), "b"(mode));

    // Clear the screen
    clearScreen();
}

void clearScreen() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
        videoMemory[i] = ' ';
        videoMemory[i+1] = 0x07; // Attribute byte (white on black)
    }
}

void writeCharacter(char character) {
    static int offset = 0;
    if (character == '\n') {
        offset = (offset / SCREEN_WIDTH + 1) * SCREEN_WIDTH;
    } else {
        videoMemory[offset++] = character;
        videoMemory[offset++] = 0x07; // Attribute byte (white on black)
    }

    // Scroll the screen if needed
    if (offset >= SCREEN_WIDTH * SCREEN_HEIGHT * 2) {
        // copy each row to the row above it
        for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i++) {
            videoMemory[i] = videoMemory[i + SCREEN_WIDTH * 2];
        }

        // Clear the last row
        for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i += 2) {
            videoMemory[i] = ' ';
            videoMemory[i+1] = 0x07; // Attribute byte (white on black)
        }

        offset -= SCREEN_WIDTH * 2;
    }
}

void writeString(const char* string) {
    for (int i = 0; string[i] != '\0'; i++) {
        writeCharacter(string[i]);
    }
}

void writeInteger(int integer) {
    if (integer < 0) {
        writeCharacter('-');
        num = -num;
    }

    char buffer[20];
    int i = 0;
    do {
        buffer[i++] = integer % 10 + '0';
        num /= 10;
    } while (num > 0) {
        for (int j = i - 1; j >= 0; j--) {
            writeCharacter(buffer[j]);
        }
    }
}