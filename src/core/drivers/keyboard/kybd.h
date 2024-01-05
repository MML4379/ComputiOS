#ifndef KYBD_H
#define KYBD_H

#include <stdint.h>
#include "../io/io.h"

// Define keyboard keys
#define KEY_A 0x1E
#define KEY_B 0x30
#define KEY_C 0x2E
#define KEY_D 0x20
#define KEY_E 0x12
#define KEY_F 0x21
#define KEY_G 0x22
#define KEY_H 0x23
#define KEY_I 0x17
#define KEY_J 0x24
#define KEY_K 0x25
#define KEY_L 0x26
#define KEY_M 0x32
#define KEY_N 0x31
#define KEY_O 0x18
#define KEY_P 0x19
#define KEY_Q 0x10
#define KEY_R 0x13
#define KEY_S 0x1F
#define KEY_T 0x14
#define KEY_U 0x16
#define KEY_V 0x2F
#define KEY_W 0x11
#define KEY_X 0x2D
#define KEY_Y 0x15
#define KEY_Z 0x2C

// Numeric keys
#define KEY_0 0x0B
#define KEY_1 0x02
#define KEY_2 0x03
#define KEY_3 0x04
#define KEY_4 0x05
#define KEY_5 0x06
#define KEY_6 0x07
#define KEY_7 0x08
#define KEY_8 0x09
#define KEY_9 0x0A

// Function keys
#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x57
#define KEY_F12 0x58

// Special keys
#define KEY_ENTER 0x1C
#define KEY_SPACE 0x39
#define KEY_BACKSPACE 0x0E
#define KEY_TAB 0x0F
#define KEY_ESC 0x01
#define KEY_CAPS_LOCK 0x3A
#define KEY_LEFT_SHIFT 0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_LEFT_CTRL 0x1D
#define KEY_RIGHT_CTRL 0x9D
#define KEY_LEFT_ALT 0x38
#define KEY_RIGHT_ALT 0xB8
#define KEY_LEFT_GUI 0x9E
#define KEY_RIGHT_GUI 0xDE
#define KEY_INSERT 0xD2
#define KEY_DELETE 0xD3
#define KEY_HOME 0xC7
#define KEY_END 0xCF
#define KEY_PAGE_UP 0xC9
#define KEY_PAGE_DOWN 0xD1
#define KEY_UP_ARROW 0xC8
#define KEY_DOWN_ARROW 0xD0
#define KEY_LEFT_ARROW 0xCB
#define KEY_RIGHT_ARROW 0xCD
#define KEY_NUM_LOCK 0x45
#define KEY_SCROLL_LOCK 0x46
#define KEY_PRINT_SCREEN 0xB7
#define KEY_PAUSE 0xC5

// Add more keys...

// Initialize the keyboard driver
void init_keyboard();

// Read a scan code from the keyboard
uint8_t readScanCode();

// Map scan code to character
char mapScanCode(uint8_t scan_code);

#endif // KYBD_H