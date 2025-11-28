#pragma once
#include "types.hpp"

void pic_remap();
void pic_unmask();
void pic_send_eoi(uint8 irq);