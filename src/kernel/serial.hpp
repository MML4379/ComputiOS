#pragma once
#include "libk/types.hpp"

void serial_init();
void serial_write_char(char c);
void serial_write_str(const char* s);