#pragma once
#include "libk/types.hpp"

void serial_init();
void serial_write_char(char c);
void serial_write_str(const char* s);
void serial_write_hex8(uint8 v);
void serial_write_hex16(uint16 v);
void serial_write_hex32(uint32 v);
void serial_write_hex64(uint64 v);