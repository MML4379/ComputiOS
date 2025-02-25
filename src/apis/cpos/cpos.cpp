// ComputiOS API
// Copyright © 2025 MML Tech LLC
// Licensed Under the ComputiOS License

// This API lets applications, subroutines, extensions, and the end user interact with the ComputiOS kernel and its drivers.

#include "cpos.hpp"

API apiError(const char[128]* error) {
    printf(error);
}

API apiLoopInit(void kernelInit, uint64_t cpuSpeed) {
    if (!kernelInit) {
        return API
    } else if (!cpuSpeed) {

    }
}