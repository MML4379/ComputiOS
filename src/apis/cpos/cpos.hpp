// ComputiOS API
// Copyright © 2025 MML Tech LLC
// Licensed Under the ComputiOS License

// This API lets applications, subroutines, extensions, and the end user interact with the ComputiOS kernel and its drivers.

#include "../../kernel/cposapi/kernel.hpp"
#include "../libc/libc.h"

// API Loop initializer
// Pass the kernel initializer to it (create ONE initializer)
// Also pass the gathered CPU speed, this will help the API be syncronized with the clock cycles.
typedef API void;
API apiLoopInit(void kernelInit, uint64_t cpuSpeed);
void apiInit();

API apiError(const char[128]* errTxt); // Error