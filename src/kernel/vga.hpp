#pragma once
#include "types.hpp"

static void putc_at(int row, int col, char c, uint8 attr); // print a character
static void print_str(int row, int col, const char* s, uint8 attr); // print a string
static void print_dec(int row, int col, uint64 x, uint8 attr); // print a decimal (ULL)
static void print_hex(int row, int col, uint64 val, uint8 attr); // print a hexadecimal value