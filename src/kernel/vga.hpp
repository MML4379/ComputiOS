#pragma once
#include "libk/types.hpp"

void putc_at(int row, int col, char c, uint8 attr);
void print_str(int row, int col, const char* s, uint8 attr);
void print_dec(int row, int col, uint64 x, uint8 attr);
void print_hex(int row, int col, uint64 val, uint8 attr);